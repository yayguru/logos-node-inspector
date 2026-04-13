#include "status_classifier.h"

#include <sstream>
#include <utility>

namespace inspector {
namespace {

std::string sshPrefix(const ConnectionProfile& profile)
{
    std::ostringstream command;
    command << "ssh";
    if (!profile.identityFile.empty()) {
        command << " -i \"" << profile.identityFile << "\"";
    }
    if (profile.port > 0 && profile.port != 22) {
        command << " -p " << profile.port;
    }
    command << " " << profile.user << "@" << profile.host;
    return command.str();
}

void addSuggestion(InspectorSnapshot& snapshot,
                   const Severity severity,
                   std::string title,
                   std::string detail,
                   std::string command)
{
    snapshot.overallStatus = worstSeverity(snapshot.overallStatus, severity);
    snapshot.suggestions.push_back(Suggestion {
        .severity = severity,
        .title = std::move(title),
        .detail = std::move(detail),
        .command = std::move(command)
    });
}

} // namespace

void StatusClassifier::apply(InspectorSnapshot& snapshot, const ConnectionProfile& profile)
{
    snapshot.suggestions.clear();
    snapshot.overallStatus = Severity::healthy;

    const auto remote = sshPrefix(profile);
    const auto apiProbe = "curl -sS --max-time 15 http://127.0.0.1:" + std::to_string(profile.apiPort)
        + "/network/info && echo && curl -sS --max-time 15 http://127.0.0.1:" + std::to_string(profile.apiPort)
        + "/cryptarchia/info";

    if (snapshot.collectorStatus == "error") {
        addSuggestion(
            snapshot,
            Severity::critical,
            "Collector failed",
            "SSH collection did not return a structured snapshot. Verify SSH access, host reachability, and the local ssh binary path.",
            remote + " \"hostname && uname -a && docker ps\""
        );
        return;
    }

    if (snapshot.hostStatus.hasDisk) {
        if (snapshot.hostStatus.diskUsedPercent >= 95.0) {
            addSuggestion(
                snapshot,
                Severity::critical,
                "Disk almost exhausted",
                "Root disk usage is at or above 95%. Logos sync and Docker health checks may stall until space is freed.",
                remote + " \"df -h / && docker system df && du -sh /root/.logos-node /var/lib/docker 2>/dev/null\""
            );
        } else if (snapshot.hostStatus.diskUsedPercent >= 85.0) {
            addSuggestion(
                snapshot,
                Severity::warning,
                "Disk pressure rising",
                "Root disk usage is above 85%. Keep an eye on Logos state growth and Docker logs before it becomes operational risk.",
                remote + " \"df -h / && docker system df\""
            );
        }
    }

    const auto* logosService = findService(snapshot, profile.logosContainerName);
    if (logosService == nullptr || !logosService->present) {
        addSuggestion(
            snapshot,
            Severity::critical,
            "logos-node container missing",
            "The expected Logos container was not found in Docker. Confirm the container name or deployment state.",
            remote + " \"docker ps -a --format '{{.Names}}\\t{{.Status}}'\""
        );
    } else {
        if (!logosService->running) {
            addSuggestion(
                snapshot,
                Severity::critical,
                "logos-node not running",
                "The Logos container exists but is not running.",
                remote + " \"docker ps -a --filter name=" + profile.logosContainerName + " && docker logs --tail 120 " + profile.logosContainerName + "\""
            );
        } else if (logosService->health == "unhealthy") {
            addSuggestion(
                snapshot,
                Severity::warning,
                "logos-node health check failing",
                "Docker reports logos-node as unhealthy. Compare the container healthcheck with the live Logos API endpoints.",
                remote + " \"docker inspect --format='{{json .State.Health}}' " + profile.logosContainerName + " && " + apiProbe + "\""
            );
        }
    }

    if (!snapshot.logosStatus.networkReachable) {
        addSuggestion(
            snapshot,
            Severity::critical,
            "network/info unavailable",
            "The Basecamp inspector could not fetch the Logos network endpoint from inside the VPS.",
            remote + " \"" + apiProbe + "\""
        );
    }

    if (!snapshot.logosStatus.cryptarchiaReachable) {
        addSuggestion(
            snapshot,
            Severity::critical,
            "cryptarchia/info unavailable",
            "The consensus endpoint is not responding. This usually means Logos is stalled or still recovering from a bad local state.",
            remote + " \"" + apiProbe + " && docker logs --tail 200 " + profile.logosContainerName + "\""
        );
    }

    if (snapshot.logosStatus.nPeers >= 0 && snapshot.logosStatus.nPeers < 3) {
        addSuggestion(
            snapshot,
            Severity::warning,
            "Low peer count",
            "The Logos node has fewer than three peers. The node may still be bootstrapping or have connectivity issues.",
            remote + " \"" + apiProbe + "\""
        );
    }

    if (snapshot.logosStatus.mode == "Bootstrapping") {
        addSuggestion(
            snapshot,
            Severity::warning,
            "Node is still bootstrapping",
            "Bootstrapping is normal while a node catches up, but operators should watch for stalled height or slot progression.",
            remote + " \"for i in 1 2 3; do date; " + apiProbe + "; echo; sleep 30; done\""
        );
    }

    const auto* wakuService = findService(snapshot, profile.wakuContainerName);
    if (wakuService == nullptr || !wakuService->present) {
        addSuggestion(
            snapshot,
            Severity::warning,
            "nwaku container missing",
            "The companion nwaku container was not found. If this host is meant to run the full Logos stack, verify that nwaku is deployed separately.",
            remote + " \"docker ps -a --format '{{.Names}}\\t{{.Status}}'\""
        );
    } else if (!wakuService->running) {
        addSuggestion(
            snapshot,
            Severity::warning,
            "nwaku not running",
            "The nwaku container is present but not running.",
            remote + " \"docker ps -a --filter name=" + profile.wakuContainerName + " && docker logs --tail 120 " + profile.wakuContainerName + "\""
        );
    }

    if (snapshot.suggestions.empty()) {
        snapshot.overallStatus = Severity::healthy;
        snapshot.suggestions.push_back(Suggestion {
            .severity = Severity::healthy,
            .title = "Read-only checks look healthy",
            .detail = "logos-node is responding, peer connectivity is present, and host pressure signals are below warning thresholds.",
            .command = remote + " \"" + apiProbe + "\""
        });
    }
}

} // namespace inspector
