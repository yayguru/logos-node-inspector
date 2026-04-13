#include "remote_output_parser.h"
#include "status_classifier.h"

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

inspector::ConnectionProfile defaultProfile()
{
    inspector::ConnectionProfile profile;
    profile.name = "Hostinger";
    profile.host = "212.227.95.210";
    profile.user = "root";
    profile.identityFile = "/home/operator/.ssh/logos";
    return profile;
}

} // namespace

int main()
{
    {
        auto snapshot = inspector::RemoteOutputParser::parse(readFixture("healthy_bootstrapping.txt"), "", 0);
        inspector::StatusClassifier::apply(snapshot, defaultProfile());
        assert(snapshot.overallStatus == inspector::Severity::warning);
        assert(!snapshot.suggestions.empty());
    }

    {
        auto snapshot = inspector::RemoteOutputParser::parse(readFixture("disk_full.txt"), "", 0);
        inspector::StatusClassifier::apply(snapshot, defaultProfile());
        assert(snapshot.overallStatus == inspector::Severity::critical);
    }

    {
        auto snapshot = inspector::RemoteOutputParser::parse(readFixture("unhealthy_logos.txt"), "", 0);
        inspector::StatusClassifier::apply(snapshot, defaultProfile());
        assert(snapshot.overallStatus == inspector::Severity::critical);
    }

    {
        auto snapshot = inspector::RemoteOutputParser::parse(readFixture("missing_nwaku.txt"), "", 0);
        inspector::StatusClassifier::apply(snapshot, defaultProfile());
        assert(snapshot.overallStatus == inspector::Severity::warning);
    }

    {
        auto snapshot = inspector::RemoteOutputParser::parse(readFixture("ssh_failure.txt"), "", 255);
        inspector::StatusClassifier::apply(snapshot, defaultProfile());
        assert(snapshot.overallStatus == inspector::Severity::critical);
        assert(!snapshot.suggestions.empty());
    }

    std::cout << "status_classifier tests passed\n";
    return 0;
}
