#pragma once

#include "droneops/Mission.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace droneops::package {

struct ManifestFile {
    std::string path;
    std::uintmax_t size{0};
    std::string hash;
};

struct PackageManifest {
    std::string package_id;
    std::string droneops_version{"0.1.0"};
    std::string camposync_contract_version{"0.1.0"};
    std::string camponode_id{"CAMPO-NODE-LOCAL"};
    std::string project_id;
    std::string campaign_id;
    std::string area_id;
    std::string mission_id;
    std::string origin_module{"droneops"};
    std::string human_responsible;
    std::string validation_status{"rascunho"};
    std::string sync_status{"nao_sincronizado"};
    std::string expected_destination{"SisTer"};
    std::vector<ManifestFile> files;
};

std::string stable_file_hash(const std::filesystem::path& path);

PackageManifest buildManifest(const std::filesystem::path& package_root,
                              const MissionRecord& mission);

void writeManifestJson(const std::filesystem::path& output,
                       const PackageManifest& manifest);

void writeMissionPackage(const std::filesystem::path& campaign_dir,
                         const std::filesystem::path& output_dir,
                         const MissionRecord& mission);

} // namespace droneops::package

