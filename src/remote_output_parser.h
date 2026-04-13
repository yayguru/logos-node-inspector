#pragma once

#include "inspector_types.h"

#include <string>

namespace inspector {

class RemoteOutputParser {
public:
    static InspectorSnapshot parse(const std::string& stdoutText,
                                   const std::string& stderrText,
                                   int exitCode);
};

} // namespace inspector

