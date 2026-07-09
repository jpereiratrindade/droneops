#pragma once

#include "droneops/Mission.hpp"
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

struct sqlite3;

namespace droneops::sqlite {

class SqliteStore {
public:
    explicit SqliteStore(const std::filesystem::path& db_path);
    ~SqliteStore();

    // Impede cópia
    SqliteStore(const SqliteStore&) = delete;
    SqliteStore& operator=(const SqliteStore&) = delete;

    void initSchema();
    void saveMission(const MissionRecord& record);
    MissionRecord getMission(const std::string& mission_id);
    std::vector<MissionRecord> getAllMissions();

private:
    sqlite3* db_{nullptr};
    std::string serializeChecklist(const std::vector<ChecklistItem>& checklist) const;
    std::vector<ChecklistItem> deserializeChecklist(const std::string& json_str) const;
};

} // namespace droneops::sqlite
