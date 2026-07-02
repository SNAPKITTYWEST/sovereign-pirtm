//===- LeanFFI.h - Lean Proof Verification Bridge -*- C++ -*-===//
//
// Bridge to Lean 4 for formal proof verification.
// Non-recursive. Every verification produces a WORM-sealed receipt.
//
//===----------------------------------------------------------------------===//

#ifndef LEAN_FFI_H
#define LEAN_FFI_H

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace pirtm {

/// Result of a Lean proof verification.
struct LeanProofResult {
    bool verified;          // true if proof succeeded
    std::string hash;       // SHA-256 of proof output
    std::string error_msg;  // Error message if verification failed
    uint64_t timestamp_ns;  // Timestamp of verification

    LeanProofResult();
};

/// Configuration for Lean FFI bridge.
struct LeanConfig {
    std::string lean_executable;    // Path to lean executable
    std::string lean_project_dir;   // Path to Lean project root
    uint64_t timeout_ms;            // Verification timeout
    bool fail_closed;               // If true, fail when lean unavailable

    LeanConfig();
    static LeanConfig defaults();
};

/// Bridge to Lean 4 proof verifier.
class LeanFFIBridge {
public:
    explicit LeanFFIBridge(const LeanConfig& config = LeanConfig::defaults());

    /// Verify a Lean proof file.
    LeanProofResult verify_proof(const std::string& lean_file_path);

    /// Verify a proof from inline Lean code.
    LeanProofResult verify_inline(const std::string& lean_code);

    /// Compute SHA-256 of proof output.
    std::string compute_proof_hash(const std::string& proof_output);

    /// Check if Lean is available.
    bool is_lean_available() const;

private:
    /// Run Lean executable on a file.
    std::optional<std::string> run_lean(const std::string& file_path);

    /// Run Lean on inline code (write to temp file).
    std::optional<std::string> run_lean_inline(const std::string& code);

    /// Get current timestamp in nanoseconds.
    uint64_t now_ns() const;

    LeanConfig config_;
};

} // namespace pirtm

#endif // LEAN_FFI_H
