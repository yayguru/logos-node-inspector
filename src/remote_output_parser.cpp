#include "remote_output_parser.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <map>
#include <optional>
#include <regex>
#include <sstream>
#include <utility>

namespace inspector {
namespace {

constexpr std::string_view kSectionStart = "__NODE_INSPECTOR_SECTION__:";
constexpr std::string_view kSectionEnd = "__NODE_INSPECTOR_END_SECTION__:";

std::string nowUtcIso8601()
{
    const auto now = std::chrono::system_clock::now();
    const auto timeValue = std::chrono::system_clock::to_time_t(now);

    std::tm tmValue {};
#if defined(_WIN32)
    gmtime_s(&tmValue, &timeValue);
#else
    gmtime_r(&timeValue, &tmValue);
#endif

    std::ostringstream stream;
    stream << std::put_time(&tmValue, "%Y-%m-%dT%H:%M:%SZ");
    return stream.str();
}

std::vector<std::string> splitLines(const std::string& text)
{
    std::vector<std::string> lines;
    std::stringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    return lines;
}

std::map<std::string, std::string> extractSections(const std::string& text)
{
    std::map<std::string, std::string> sections;
    std::optional<std::string> currentName;
    std::ostringstream currentContent;

    for (const auto& rawLine : splitLines(text)) {
        if (rawLine.rfind(kSectionStart.data(), 0) == 0) {
            if (currentName.has_value()) {
                sections[*currentName] = currentContent.str();
                currentContent.str("");
                currentContent.clear();
            }
            currentName = rawLine.substr(kSectionStart.size());
            continue;
        }

        if (rawLine.rfind(kSectionEnd.data(), 0) == 0) {
            const auto closingName = rawLine.substr(kSectionEnd.size());
            if (currentName.has_value() && *currentName == closingName) {
                sections[*currentName] = currentContent.str();
            }
            currentName.reset();
            currentContent.str("");
            currentContent.clear();
            continue;
        }

        if (currentName.has_value()) {
            currentContent << rawLine << '\n';
        }
    }

    if (currentName.has_value()) {
        sections[*currentName] = currentContent.str();
    }

    return sections;
}

std::optional<std::string> extractJsonString(const std::string& json, const std::string& key)
{
    const std::regex pattern("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch match;
    if (std::regex_search(json, match, pattern) && match.size() > 1) {
        return match[1].str();
    }
    return std::nullopt;
}

std::optional<long long> extractJsonInteger(const std::string& json, const std::string& key)
{
    const std::regex pattern("\"" + key + "\"\\s*:\\s*(-?[0-9]+)");
    std::smatch match;
    if (std::regex_search(json, match, pattern) && match.size() > 1) {
        return std::stoll(match[1].str());
    }
    return std::nullopt;
}

void parseHostBasics(HostStatus& hostStatus, const std::string& section)
{
    const auto lines = splitLines(section);
    if (!lines.empty()) {
        hostStatus.hostname = trim(lines[0]);
    }
    if (lines.size() > 1) {
        hostStatus.kernel = trim(lines[1]);
    }
    if (lines.size() > 2) {
        hostStatus.uptime = trim(lines[2]);
    }
}

void parseRootDf(HostStatus& hostStatus, const std::string& section)
{
    const auto lines = splitLines(section);
    if (lines.size() < 2) {
        return;
    }

    std::stringstream row(lines[1]);
    std::string filesystem;
    std::string mountedOn;
    std::string percentText;

    row >> filesystem >> hostStatus.diskTotalBytes >> hostStatus.diskUsedBytes >> hostStatus.diskAvailableBytes >> percentText >> mountedOn;

    if (!filesystem.empty()) {
        hostStatus.hasDisk = true;
    }

    percentText = trim(percentText);
    if (!percentText.empty() && percentText.back() == '%') {
        percentText.pop_back();
    }
    if (!percentText.empty()) {
        hostStatus.diskUsedPercent = std::stod(percentText);
    }
}

void parseMemory(HostStatus& hostStatus, const std::string& section)
{
    for (const auto& line : splitLines(section)) {
        const auto trimmed = trim(line);
        if (trimmed.rfind("Mem:", 0) != 0) {
            continue;
        }

        std::stringstream row(trimmed);
        std::string label;
        std::int64_t shared = -1;
        std::int64_t buffers = -1;
        row >> label
            >> hostStatus.memoryTotalBytes
            >> hostStatus.memoryUsedBytes
            >> hostStatus.memoryFreeBytes
            >> shared
            >> buffers
            >> hostStatus.memoryAvailableBytes;
        hostStatus.hasMemory = true;
        return;
    }
}

void parseDockerPs(InspectorSnapshot& snapshot, const std::string& section)
{
    for (const auto& line : splitLines(section)) {
        if (trim(line).empty()) {
            continue;
        }

        std::stringstream parts(line);
        std::string name;
        std::string image;
        std::string status;
        std::string ports;

        std::getline(parts, name, '|');
        std::getline(parts, image, '|');
        std::getline(parts, status, '|');
        std::getline(parts, ports);

        if (name.empty()) {
            continue;
        }

        auto& service = ensureService(snapshot, trim(name));
        service.present = true;
        service.image = trim(image);
        service.status = trim(status);
        service.ports = trim(ports);
        service.running = service.status.find("Up") != std::string::npos;
    }
}

void parseServiceSection(ServiceStatus& service, const std::string& section)
{
    if (trim(section) == "__MISSING__") {
        service.present = false;
        service.running = false;
        service.status = "missing";
        return;
    }

    service.present = true;
    for (const auto& line : splitLines(section)) {
        const auto separator = line.find('=');
        if (separator == std::string::npos) {
            continue;
        }

        const auto key = trim(line.substr(0, separator));
        const auto value = trim(line.substr(separator + 1));

        if (key == "name") {
            service.name = value;
        } else if (key == "running") {
            service.running = value == "true";
        } else if (key == "health") {
            service.health = value;
        } else if (key == "restart") {
            service.restartPolicy = value;
        } else if (key == "image") {
            service.image = value;
        } else if (key == "ports") {
            service.ports = value;
        } else if (key == "status") {
            service.status = value;
        }
    }
}

void parseNetworkInfo(LogosStatus& logosStatus, const std::string& section)
{
    const auto trimmed = trim(section);
    if (trimmed.rfind("__ERROR__:", 0) == 0) {
        logosStatus.networkReachable = false;
        logosStatus.networkError = trimmed;
        return;
    }

    logosStatus.networkReachable = !trimmed.empty();
    logosStatus.networkError.clear();

    if (auto value = extractJsonString(trimmed, "peer_id")) {
        logosStatus.peerId = *value;
    }
    if (auto value = extractJsonInteger(trimmed, "n_peers")) {
        logosStatus.nPeers = static_cast<int>(*value);
    }
    if (auto value = extractJsonInteger(trimmed, "n_connections")) {
        logosStatus.nConnections = static_cast<int>(*value);
    }
    if (auto value = extractJsonInteger(trimmed, "n_pending_connections")) {
        logosStatus.nPendingConnections = static_cast<int>(*value);
    }
}

void parseCryptarchiaInfo(LogosStatus& logosStatus, const std::string& section)
{
    const auto trimmed = trim(section);
    if (trimmed.rfind("__ERROR__:", 0) == 0) {
        logosStatus.cryptarchiaReachable = false;
        logosStatus.cryptarchiaError = trimmed;
        return;
    }

    logosStatus.cryptarchiaReachable = !trimmed.empty();
    logosStatus.cryptarchiaError.clear();

    if (auto value = extractJsonString(trimmed, "mode")) {
        logosStatus.mode = *value;
    }
    if (auto value = extractJsonString(trimmed, "lib")) {
        logosStatus.lib = *value;
    }
    if (auto value = extractJsonString(trimmed, "tip")) {
        logosStatus.tip = *value;
    }
    if (auto value = extractJsonInteger(trimmed, "height")) {
        logosStatus.height = *value;
    }
    if (auto value = extractJsonInteger(trimmed, "slot")) {
        logosStatus.slot = *value;
    }
}

void pushLog(InspectorSnapshot& snapshot, const std::string& serviceName, const std::string& section)
{
    snapshot.logExcerpts.push_back(LogExcerpt {
        .serviceName = serviceName,
        .content = trim(section)
    });
}

} // namespace

InspectorSnapshot RemoteOutputParser::parse(const std::string& stdoutText,
                                            const std::string& stderrText,
                                            const int exitCode)
{
    InspectorSnapshot snapshot;
    snapshot.collectedAt = nowUtcIso8601();

    const auto sections = extractSections(stdoutText);
    if (sections.empty()) {
        snapshot.collectorStatus = "error";
        snapshot.overallStatus = Severity::critical;
        snapshot.collectorMessage = trim(!stderrText.empty() ? stderrText : stdoutText);
        if (snapshot.collectorMessage.empty()) {
            snapshot.collectorMessage = "Collector returned no structured sections.";
        }
        if (exitCode != 0) {
            snapshot.collectorMessage += " (exit code " + std::to_string(exitCode) + ")";
        }
        return snapshot;
    }

    snapshot.collectorStatus = exitCode == 0 ? "ok" : "warning";
    snapshot.collectorMessage = exitCode == 0
        ? "Snapshot collected successfully."
        : "Collector returned a partial snapshot.";

    if (const auto iterator = sections.find("HOST_BASICS"); iterator != sections.end()) {
        parseHostBasics(snapshot.hostStatus, iterator->second);
    }
    if (const auto iterator = sections.find("ROOT_DF"); iterator != sections.end()) {
        parseRootDf(snapshot.hostStatus, iterator->second);
    }
    if (const auto iterator = sections.find("MEMORY"); iterator != sections.end()) {
        parseMemory(snapshot.hostStatus, iterator->second);
    }
    if (const auto iterator = sections.find("DOCKER_SYSTEM_DF"); iterator != sections.end()) {
        snapshot.hostStatus.dockerDiskSummary = trim(iterator->second);
    }
    if (const auto iterator = sections.find("DOCKER_PS"); iterator != sections.end()) {
        parseDockerPs(snapshot, iterator->second);
    }
    if (const auto iterator = sections.find("SERVICE_LOGOS_NODE"); iterator != sections.end()) {
        auto& service = ensureService(snapshot, "logos-node");
        parseServiceSection(service, iterator->second);
    }
    if (const auto iterator = sections.find("SERVICE_NWAKU"); iterator != sections.end()) {
        auto& service = ensureService(snapshot, "nwaku");
        parseServiceSection(service, iterator->second);
    }
    if (const auto iterator = sections.find("NETWORK_INFO"); iterator != sections.end()) {
        parseNetworkInfo(snapshot.logosStatus, iterator->second);
    }
    if (const auto iterator = sections.find("CRYPTARCHIA_INFO"); iterator != sections.end()) {
        parseCryptarchiaInfo(snapshot.logosStatus, iterator->second);
    }
    if (const auto iterator = sections.find("LOGOS_NODE_LOGS"); iterator != sections.end()) {
        pushLog(snapshot, "logos-node", iterator->second);
    }
    if (const auto iterator = sections.find("NWAKU_LOGS"); iterator != sections.end()) {
        pushLog(snapshot, "nwaku", iterator->second);
    }

    return snapshot;
}

} // namespace inspector
