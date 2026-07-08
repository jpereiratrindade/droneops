#pragma once

#include "droneops/Mission.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace droneops::csv {

struct ReadResult {
    std::vector<MissionRecord> records;
    std::vector<ValidationIssue> issues;
};

std::vector<std::string> defaultMissionHeaders();
std::vector<std::string> parseCsvLine(const std::string& line);

ReadResult readMissions(const std::filesystem::path& input);

void writeMissionsCsv(const std::filesystem::path& output,
                      const std::vector<MissionRecord>& records);

void writeMissionsJsonl(const std::filesystem::path& output,
                        const std::vector<MissionRecord>& records);

void writeTemplateCsv(const std::filesystem::path& output);

} // namespace droneops::csv

