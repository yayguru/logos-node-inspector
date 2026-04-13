#pragma once

#include "inspector_types.h"

namespace inspector {

class StatusClassifier {
public:
    static void apply(InspectorSnapshot& snapshot, const ConnectionProfile& profile);
};

} // namespace inspector

