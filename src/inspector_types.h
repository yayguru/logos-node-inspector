#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace inspector {

enum class Severity {
    healthy,
    warning,
    critical
};

struct ConnectionProfile {
    std::string name = "Primary VPS";
    std::string host;
    int port = 22;
    std::string user = "root";
    std::string identityFile;
    bool useSudo = false;
    std::string logosContainerName = "logos-node";
    std::string wakuContainerName = "nwaku";
    int apiPort = 8080;
};

struct HostStatus {
    std::string hostname;
    std::string kernel;
    std::string uptime;

    bool hasDisk = false;
    std::int64_t diskTotalBytes = -1;
    std::int64_t diskUsedBytes = -1;
    std::int64_t diskAvailableBytes = -1;
    double diskUsedPercent = -1.0;

    bool hasMemory = false;
    std::int64_t memoryTotalBytes = -1;
    std::int64_t memoryUsedBytes = -1;
    std::int64_t memoryFreeBytes = -1;
    std::int64_t memoryAvailableBytes = -1;

    std::string dockerDiskSummary;
};

struct LogosStatus {
    bool networkReachable = false;
    bool cryptarchiaReachable = false;

    std::string networkError;
    std::string cryptarchiaError;

    std::string peerId;
    int nPeers = -1;
    int nConnections = -1;
    int nPendingConnections = -1;

    std::string mode;
    std::int64_t height = -1;
    std::int64_t slot = -1;
    std::string lib;
    std::string tip;
};

struct ServiceStatus {
    std::string name;
    bool present = false;
    bool running = false;
    std::string status;
    std::string health = "unknown";
    std::string restartPolicy;
    std::string image;
    std::string ports;
};

struct LogExcerpt {
    std::string serviceName;
    std::string content;
};

struct Suggestion {
    Severity severity = Severity::warning;
    std::string title;
    std::string detail;
    std::string command;
};

struct InspectorSnapshot {
    std::string collectedAt;
    std::string collectorStatus = "idle";
    std::string collectorMessage;
    Severity overallStatus = Severity::warning;
    HostStatus hostStatus;
    LogosStatus logosStatus;
    std::vector<ServiceStatus> serviceStatuses;
    std::vector<LogExcerpt> logExcerpts;
    std::vector<Suggestion> suggestions;
};

inline constexpr std::string_view severityToString(Severity severity)
{
    switch (severity) {
    case Severity::healthy:
        return "healthy";
    case Severity::warning:
        return "warning";
    case Severity::critical:
        return "critical";
    }

    return "warning";
}

inline Severity worstSeverity(Severity left, Severity right)
{
    if (left == Severity::critical || right == Severity::critical) {
        return Severity::critical;
    }

    if (left == Severity::warning || right == Severity::warning) {
        return Severity::warning;
    }

    return Severity::healthy;
}

inline std::string trim(std::string value)
{
    auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

inline ServiceStatus* findService(InspectorSnapshot& snapshot, const std::string& name)
{
    auto iterator = std::find_if(snapshot.serviceStatuses.begin(), snapshot.serviceStatuses.end(),
        [&](const ServiceStatus& service) { return service.name == name; });
    if (iterator == snapshot.serviceStatuses.end()) {
        return nullptr;
    }
    return &(*iterator);
}

inline const ServiceStatus* findService(const InspectorSnapshot& snapshot, const std::string& name)
{
    auto iterator = std::find_if(snapshot.serviceStatuses.begin(), snapshot.serviceStatuses.end(),
        [&](const ServiceStatus& service) { return service.name == name; });
    if (iterator == snapshot.serviceStatuses.end()) {
        return nullptr;
    }
    return &(*iterator);
}

inline ServiceStatus& ensureService(InspectorSnapshot& snapshot, const std::string& name)
{
    if (auto* existing = findService(snapshot, name)) {
        return *existing;
    }

    snapshot.serviceStatuses.push_back(ServiceStatus{ .name = name });
    return snapshot.serviceStatuses.back();
}

} // namespace inspector

