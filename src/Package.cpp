#include "droneops/Package.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <cstring>

namespace droneops::package {
namespace {

std::string jsonEscape(const std::string& value) {
    std::string out;
    for (unsigned char c : value) {
        switch (c) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            out.push_back(static_cast<char>(c));
        }
    }
    return out;
}

void writeTextFile(const std::filesystem::path& path, const std::string& text) {
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("nao foi possivel escrever arquivo: " + path.string());
    }
    out << text;
}

std::string missionJson(const MissionRecord& mission) {
    std::ostringstream out;
    out << "{\n"
        << "  \"project_id\": \"" << jsonEscape(mission.project_id) << "\",\n"
        << "  \"campaign_id\": \"" << jsonEscape(mission.campaign_id) << "\",\n"
        << "  \"area_id\": \"" << jsonEscape(mission.area_id) << "\",\n"
        << "  \"mission_id\": \"" << jsonEscape(mission.mission_id) << "\",\n"
        << "  \"objective\": \"" << jsonEscape(mission.objective) << "\",\n"
        << "  \"planned_date\": \"" << jsonEscape(mission.planned_date) << "\",\n"
        << "  \"responsible\": \"" << jsonEscape(mission.responsible) << "\",\n"
        << "  \"aircraft_id\": \"" << jsonEscape(mission.aircraft_id) << "\",\n"
        << "  \"sensor_id\": \"" << jsonEscape(mission.sensor_id) << "\",\n"
        << "  \"authorization_ref\": \"" << jsonEscape(mission.authorization_ref) << "\",\n"
        << "  \"operator_id\": \"" << jsonEscape(mission.operator_id) << "\",\n"
        << "  \"drone_qr\": \"" << jsonEscape(mission.drone_qr) << "\",\n"
        << "  \"aro_ref\": \"" << jsonEscape(mission.aro_ref) << "\",\n"
        << "  \"gps_latitude\": \"" << jsonEscape(mission.gps_latitude) << "\",\n"
        << "  \"gps_longitude\": \"" << jsonEscape(mission.gps_longitude) << "\",\n"
        << "  \"gps_accuracy_m\": \"" << jsonEscape(mission.gps_accuracy_m) << "\",\n"
        << "  \"photo_paths\": \"" << jsonEscape(mission.photo_paths) << "\",\n"
        << "  \"final_decision\": \"" << jsonEscape(mission.final_decision) << "\",\n"
        << "  \"status\": \"" << to_string(calculate_status(mission)) << "\"\n"
        << "}\n";
    return out.str();
}

std::string protocolCertificateJson(const MissionRecord& mission) {
    std::ostringstream out;
    out << "{\n"
        << "  \"schema\": \"droneops.protocol_certificate.v1\",\n"
        << "  \"mission_id\": \"" << jsonEscape(mission.mission_id) << "\",\n"
        << "  \"operator_id\": \"" << jsonEscape(mission.operator_id.empty() ? mission.responsible : mission.operator_id) << "\",\n"
        << "  \"drone_qr\": \"" << jsonEscape(mission.drone_qr.empty() ? mission.aircraft_id : mission.drone_qr) << "\",\n"
        << "  \"aro_ref\": \"" << jsonEscape(mission.aro_ref) << "\",\n"
        << "  \"latitude\": \"" << jsonEscape(mission.gps_latitude) << "\",\n"
        << "  \"longitude\": \"" << jsonEscape(mission.gps_longitude) << "\",\n"
        << "  \"accuracy_m\": \"" << jsonEscape(mission.gps_accuracy_m) << "\",\n"
        << "  \"photo_paths\": \"" << jsonEscape(mission.photo_paths) << "\",\n"
        << "  \"final_decision\": \"" << jsonEscape(mission.final_decision.empty() ? "rascunho" : mission.final_decision) << "\"\n"
        << "}\n";
    return out.str();
}

} // namespace

// --- Simple SHA-256 Implementation ---
class SHA256 {
public:
    SHA256() { reset(); }
    void update(const unsigned char* data, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            data_[datalen_] = data[i];
            datalen_++;
            if (datalen_ == 64) {
                transform();
                bitlen_ += 512;
                datalen_ = 0;
            }
        }
    }
    std::string digest() {
        unsigned char hash[32];
        unsigned int i = datalen_;
        if (datalen_ < 56) {
            data_[i++] = 0x80;
            while (i < 56) data_[i++] = 0x00;
        } else {
            data_[i++] = 0x80;
            while (i < 64) data_[i++] = 0x00;
            transform();
            memset(data_, 0, 56);
        }
        bitlen_ += datalen_ * 8;
        data_[63] = bitlen_; data_[62] = bitlen_ >> 8; data_[61] = bitlen_ >> 16; data_[60] = bitlen_ >> 24;
        data_[59] = bitlen_ >> 32; data_[58] = bitlen_ >> 40; data_[57] = bitlen_ >> 48; data_[56] = bitlen_ >> 56;
        transform();
        for (i = 0; i < 4; ++i) {
            hash[i]      = (state_[0] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 4]  = (state_[1] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 8]  = (state_[2] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 12] = (state_[3] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 16] = (state_[4] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 20] = (state_[5] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 24] = (state_[6] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 28] = (state_[7] >> (24 - i * 8)) & 0x000000ff;
        }
        std::ostringstream oss;
        for (i = 0; i < 32; ++i) oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        return oss.str();
    }
private:
    unsigned char data_[64];
    unsigned int datalen_;
    unsigned long long bitlen_;
    unsigned int state_[8];
    uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
    uint32_t choose(uint32_t e, uint32_t f, uint32_t g) { return (e & f) ^ (~e & g); }
    uint32_t majority(uint32_t a, uint32_t b, uint32_t c) { return (a & (b | c)) | (b & c); }
    uint32_t sig0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    uint32_t sig1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }
    void transform() {
        uint32_t m[64];
        for (int i = 0, j = 0; i < 16; ++i, j += 4)
            m[i] = (data_[j] << 24) | (data_[j + 1] << 16) | (data_[j + 2] << 8) | (data_[j + 3]);
        for (int i = 16; i < 64; ++i)
            m[i] = sig1(m[i - 2]) + m[i - 7] + sig0(m[i - 15]) + m[i - 16];
        uint32_t a = state_[0], b = state_[1], c = state_[2], d = state_[3];
        uint32_t e = state_[4], f = state_[5], g = state_[6], h = state_[7];
        const uint32_t k[64] = {
            0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
            0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
            0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
            0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
            0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
            0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
            0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
            0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
        };
        for (int i = 0; i < 64; ++i) {
            uint32_t t1 = h + (rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25)) + choose(e, f, g) + k[i] + m[i];
            uint32_t t2 = (rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22)) + majority(a, b, c);
            h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        }
        state_[0] += a; state_[1] += b; state_[2] += c; state_[3] += d;
        state_[4] += e; state_[5] += f; state_[6] += g; state_[7] += h;
    }
    void reset() {
        datalen_ = 0; bitlen_ = 0;
        state_[0] = 0x6a09e667; state_[1] = 0xbb67ae85; state_[2] = 0x3c6ef372; state_[3] = 0xa54ff53a;
        state_[4] = 0x510e527f; state_[5] = 0x9b05688c; state_[6] = 0x1f83d9ab; state_[7] = 0x5be0cd19;
    }
};

std::string stable_file_hash(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("nao foi possivel abrir arquivo para hash: " + path.string());
    }

    SHA256 sha;
    char buf[4096];
    while (in.read(buf, sizeof(buf))) {
        sha.update(reinterpret_cast<const unsigned char*>(buf), in.gcount());
    }
    if (in.gcount() > 0) {
        sha.update(reinterpret_cast<const unsigned char*>(buf), in.gcount());
    }
    return "sha256:" + sha.digest();
}

PackageManifest buildManifest(const std::filesystem::path& package_root,
                              const MissionRecord& mission) {
    PackageManifest manifest;
    manifest.package_id = "PKG-DRONEOPS-" + mission.mission_id;
    manifest.project_id = mission.project_id;
    manifest.campaign_id = mission.campaign_id;
    manifest.area_id = mission.area_id;
    manifest.mission_id = mission.mission_id;
    manifest.human_responsible = mission.responsible;
    manifest.validation_status = has_open_occurrence(mission) ? "bloqueado" : "validado";

    for (const auto& entry : std::filesystem::recursive_directory_iterator(package_root)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto relative = std::filesystem::relative(entry.path(), package_root).generic_string();
        if (relative == "manifest.json") {
            continue;
        }
        manifest.files.push_back({relative, entry.file_size(), stable_file_hash(entry.path())});
    }
    return manifest;
}

void writeManifestJson(const std::filesystem::path& output,
                       const PackageManifest& manifest) {
    if (output.has_parent_path()) {
        std::filesystem::create_directories(output.parent_path());
    }
    std::ofstream out(output);
    if (!out) {
        throw std::runtime_error("nao foi possivel escrever manifesto: " + output.string());
    }

    out << "{\n"
        << "  \"package_id\": \"" << jsonEscape(manifest.package_id) << "\",\n"
        << "  \"droneops_version\": \"" << jsonEscape(manifest.droneops_version) << "\",\n"
        << "  \"camposync_contract_version\": \"" << jsonEscape(manifest.camposync_contract_version) << "\",\n"
        << "  \"camponode_id\": \"" << jsonEscape(manifest.camponode_id) << "\",\n"
        << "  \"project_id\": \"" << jsonEscape(manifest.project_id) << "\",\n"
        << "  \"campaign_id\": \"" << jsonEscape(manifest.campaign_id) << "\",\n"
        << "  \"area_id\": \"" << jsonEscape(manifest.area_id) << "\",\n"
        << "  \"mission_id\": \"" << jsonEscape(manifest.mission_id) << "\",\n"
        << "  \"origin_module\": \"" << jsonEscape(manifest.origin_module) << "\",\n"
        << "  \"human_responsible\": \"" << jsonEscape(manifest.human_responsible) << "\",\n"
        << "  \"validation_status\": \"" << jsonEscape(manifest.validation_status) << "\",\n"
        << "  \"sync_status\": \"" << jsonEscape(manifest.sync_status) << "\",\n"
        << "  \"expected_destination\": \"" << jsonEscape(manifest.expected_destination) << "\",\n"
        << "  \"files\": [\n";

    for (std::size_t i = 0; i < manifest.files.size(); ++i) {
        const auto& file = manifest.files[i];
        out << "    {\"path\": \"" << jsonEscape(file.path) << "\", \"size\": " << file.size
            << ", \"hash\": \"" << jsonEscape(file.hash) << "\"}";
        if (i + 1 < manifest.files.size()) {
            out << ',';
        }
        out << '\n';
    }
    out << "  ]\n}\n";
}

void writeMissionPackage(const std::filesystem::path& campaign_dir,
                         const std::filesystem::path& output_dir,
                         const MissionRecord& mission) {
    std::filesystem::create_directories(output_dir / "droneops" / "checklists");
    std::filesystem::create_directories(output_dir / "droneops" / "mapas");
    std::filesystem::create_directories(output_dir / "droneops" / "planos_voo");
    std::filesystem::create_directories(output_dir / "droneops" / "logs_voo");
    std::filesystem::create_directories(output_dir / "evidencias" / "documentos");
    std::filesystem::create_directories(output_dir / "logs");

    writeTextFile(output_dir / "metadata.json",
                  "{\n  \"module\": \"droneops\",\n  \"source_dir\": \""
                      + jsonEscape(campaign_dir.string()) + "\"\n}\n");
    writeTextFile(output_dir / "droneops" / ("missao_" + mission.mission_id + ".json"),
                  missionJson(mission));
    writeTextFile(output_dir / "droneops" / ("certificado_protocolo_" + mission.mission_id + ".json"),
                  protocolCertificateJson(mission));
    writeTextFile(output_dir / "logs" / "package.log",
                  "pacote CampoSync gerado para missao " + mission.mission_id + "\n");

    const auto manifest = buildManifest(output_dir, mission);
    writeManifestJson(output_dir / "manifest.json", manifest);
}

} // namespace droneops::package
