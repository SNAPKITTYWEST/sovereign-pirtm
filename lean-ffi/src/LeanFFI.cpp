//===- LeanFFI.cpp - Lean Proof Verification Implementation ===//
//
// Non-recursive Lean FFI bridge.
// Every verification is WORM-sealed with SHA-256 hash.
//
//===----------------------------------------------------------------------===//

#include "pirtm/LeanFFI.h"
#include "pirtm/ContractivityReceipt.h"
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace pirtm {

// --- LeanProofResult ---

LeanProofResult::LeanProofResult()
    : verified(false), hash(""), error_msg(""), timestamp_ns(0) {}

// --- LeanConfig ---

LeanConfig::LeanConfig()
    : lean_executable("lean"), lean_project_dir("."),
      timeout_ms(30000), fail_closed(true) {}

LeanConfig LeanConfig::defaults() {
    return LeanConfig();
}

// --- LeanFFIBridge ---

LeanFFIBridge::LeanFFIBridge(const LeanConfig& config) : config_(config) {}

LeanProofResult LeanFFIBridge::verify_proof(const std::string& lean_file_path) {
    LeanProofResult result;
    result.timestamp_ns = now_ns();

    // Check file exists
    if (!std::filesystem::exists(lean_file_path)) {
        result.verified = false;
        result.error_msg = "Lean file not found: " + lean_file_path;
        result.hash = sha256_hex(result.error_msg);
        return result;
    }

    // Check if lean is available
    if (!is_lean_available()) {
        if (config_.fail_closed) {
            result.verified = false;
            result.error_msg = "Lean not available (fail-closed mode)";
            result.hash = sha256_hex(result.error_msg);
            return result;
        }
        // Fallback: hash the file content as placeholder
        std::ifstream file(lean_file_path);
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        result.verified = true;
        result.hash = sha256_hex(content);
        return result;
    }

    // Run lean --run
    auto output = run_lean(lean_file_path);
    if (output.has_value()) {
        result.verified = true;
        result.hash = compute_proof_hash(*output);
    } else {
        result.verified = false;
        result.error_msg = "Lean verification failed";
        result.hash = sha256_hex(result.error_msg);
    }

    return result;
}

LeanProofResult LeanFFIBridge::verify_inline(const std::string& lean_code) {
    LeanProofResult result;
    result.timestamp_ns = now_ns();

    if (!is_lean_available()) {
        if (config_.fail_closed) {
            result.verified = false;
            result.error_msg = "Lean not available (fail-closed mode)";
            result.hash = sha256_hex(result.error_msg);
            return result;
        }
        result.verified = true;
        result.hash = sha256_hex(lean_code);
        return result;
    }

    auto output = run_lean_inline(lean_code);
    if (output.has_value()) {
        result.verified = true;
        result.hash = compute_proof_hash(*output);
    } else {
        result.verified = false;
        result.error_msg = "Lean inline verification failed";
        result.hash = sha256_hex(result.error_msg);
    }

    return result;
}

std::string LeanFFIBridge::compute_proof_hash(const std::string& proof_output) {
    return sha256_hex(proof_output);
}

bool LeanFFIBridge::is_lean_available() const {
    int ret = std::system((config_.lean_executable + " --version > /dev/null 2>&1").c_str());
    return ret == 0;
}

std::optional<std::string> LeanFFIBridge::run_lean(const std::string& file_path) {
    std::string cmd = config_.lean_executable + " --run " + file_path + " 2>&1";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return std::nullopt;

    std::string result;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }

    int status = pclose(pipe);
    if (status != 0) return std::nullopt;

    return result;
}

std::optional<std::string> LeanFFIBridge::run_lean_inline(const std::string& code) {
    // Write to temp file
    std::string tmp_path = std::tmpnam(nullptr);
    tmp_path += ".lean";

    std::ofstream file(tmp_path);
    file << code;
    file.close();

    auto result = run_lean(tmp_path);

    // Cleanup
    std::filesystem::remove(tmp_path);

    return result;
}

uint64_t LeanFFIBridge::now_ns() const {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count();
}

} // namespace pirtm
