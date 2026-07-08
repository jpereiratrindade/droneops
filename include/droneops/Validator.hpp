#pragma once

#include "droneops/Mission.hpp"

#include <vector>

namespace droneops::validator {

std::vector<ValidationIssue> validateMissions(const std::vector<MissionRecord>& records);

} // namespace droneops::validator

