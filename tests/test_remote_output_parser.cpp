#include "remote_output_parser.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

std::string readFixture(const std::string& fileName)
{
    std::ifstream stream(std::string(NODE_INSPECTOR_FIXTURE_DIR) + "/" + fileName);
    assert(stream.good());
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

} // namespace

int main()
{
    {
        const auto snapshot = inspector::RemoteOutputParser::parse(readFixture("healthy_bootstrapping.txt"), "", 0);
        assert(snapshot.collectorStatus == "ok");
        assert(snapshot.hostStatus.hostname == "ubuntu");
        assert(snapshot.hostStatus.hasDisk);
        assert(snapshot.hostStatus.diskUsedPercent == 59.0);
        assert(snapshot.logosStatus.networkReachable);
        assert(snapshot.logosStatus.cryptarchiaReachable);
        assert(snapshot.logosStatus.nPeers == 56);
        assert(snapshot.logosStatus.height == 20955);
        assert(snapshot.logosStatus.mode == "Bootstrapping");
        const auto* logos = inspector::findService(snapshot, "logos-node");
        assert(logos != nullptr);
        assert(logos->present);
        assert(logos->running);
        assert(logos->health == "healthy");
    }

    {
        const auto snapshot = inspector::RemoteOutputParser::parse(readFixture("missing_nwaku.txt"), "", 0);
        const auto* waku = inspector::findService(snapshot, "nwaku");
        assert(waku != nullptr);
        assert(!waku->present);
        assert(!waku->running);
        assert(snapshot.logosStatus.mode == "Online");
    }

    {
        const auto snapshot = inspector::RemoteOutputParser::parse(readFixture("ssh_failure.txt"), "", 255);
        assert(snapshot.collectorStatus == "error");
        assert(snapshot.overallStatus == inspector::Severity::critical);
        assert(!snapshot.collectorMessage.empty());
    }

    std::cout << "remote_output_parser tests passed\n";
    return 0;
}

