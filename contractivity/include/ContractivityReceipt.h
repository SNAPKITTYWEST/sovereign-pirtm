//===- ContractivityReceipt.h - Mathematical Soundness Proof -*- C++ -*-===//
//
// Generates cryptographic receipts proving mathematical soundness of
// each operator. Non-recursive. WORM-sealed.
//
//===----------------------------------------------------------------------===//

#ifndef CONTRACTIVITY_RECEIPT_H
#define CONTRACTIVITY_RECEIPT_H

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>

namespace pirtm {

/// A cryptographic receipt proving mathematical soundness.
struct ContractivityReceipt {
    uint64_t prime_index;
    std::string hash;           // SHA-256 hex string (64 chars)
    uint64_t timestamp_ns;      // Nanoseconds since epoch
    std::string operator_name;  // e.g., "operator_atom", "binary_add"
    std::string merkle_root;    // Merkle root of dependency chain

    /// Verify this receipt matches expected values.
    bool verify(uint64_t expected_prime, const std::string& expected_op) const;

    /// Serialize to JSON string.
    std::string to_json() const;

    /// Deserialize from JSON string.
    static ContractivityReceipt from_json(const std::string& json);
};

/// Receipt generator for contractivity proofs.
class ReceiptGenerator {
public:
    ReceiptGenerator();

    /// Generate a receipt for an operator atom.
    ContractivityReceipt generate_operator_atom(uint64_t prime_index);

    /// Generate a receipt for a binary operation.
    ContractivityReceipt generate_binary_op(
        const std::string& op_kind,
        uint64_t left_prime,
        uint64_t right_prime,
        uint64_t result_prime
    );

    /// Generate a receipt for a stratum boundary.
    ContractivityReceipt generate_stratum_boundary(uint64_t inner_prime);

    /// Generate a receipt for a successor operation.
    ContractivityReceipt generate_successor(uint64_t inner_prime);

    /// Verify a receipt chain (each receipt's merkle_root covers the previous).
    bool verify_chain(const std::vector<ContractivityReceipt>& chain) const;

private:
    /// Compute SHA-256 hash of data.
    std::string compute_sha256(const std::vector<uint8_t>& data) const;

    /// Compute Merkle root of receipt chain.
    std::string compute_merkle_root(const std::vector<ContractivityReceipt>& chain) const;

    /// Get current timestamp in nanoseconds.
    uint64_t now_ns() const;

    uint64_t counter_;
};

/// Compute SHA-256 of arbitrary bytes (hex string output).
std::string sha256_hex(const std::vector<uint8_t>& data);

/// Compute SHA-256 of a string.
std::string sha256_hex(const std::string& data);

} // namespace pirtm

#endif // CONTRACTIVITY_RECEIPT_H
