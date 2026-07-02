//===- PirtmLowering.h - MLIR to LLVM/WASM Lowering -*- C++ -*-===//
//
// Lowering pipeline from PIRTM dialect to LLVM IR and WebAssembly.
// Non-recursive. Every lowering produces a WORM-sealed receipt.
//
//===----------------------------------------------------------------------===//

#ifndef PIRTM_LOWERING_H
#define PIRTM_LOWERING_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace mlir {
class ModuleOp;
class MLIRContext;
} // namespace mlir

namespace pirtm {

/// Lowering target.
enum class LoweringTarget {
    LLVM,   // LLVM IR
    WASM    // WebAssembly
};

/// Configuration for the lowering pipeline.
struct LoweringConfig {
    LoweringTarget target;
    bool verify_diagnostics;    // Run verification after lowering
    bool emit_early_assembler;  // Emit assembly before final codegen
    std::string target_triple;  // Target triple for LLVM

    LoweringConfig();
    static LoweringConfig for_llvm();
    static LoweringConfig for_wasm();
};

/// Result of a lowering operation.
struct LoweringResult {
    bool success;
    std::string output;         // Generated code (LLVM IR or WASM text)
    std::string error_msg;      // Error message if lowering failed
    std::string receipt_hash;   // SHA-256 of the output
    uint64_t timestamp_ns;

    LoweringResult();
};

/// Lowering pipeline for PIRTM dialect.
class PirtmLoweringPipeline {
public:
    explicit PirtmLoweringPipeline(const LoweringConfig& config = LoweringConfig::for_llvm());

    /// Lower PIRTM MLIR to target code.
    LoweringResult lower(mlir::ModuleOp module);

    /// Lower from MLIR text.
    LoweringResult lower_from_text(const std::string& mlir_text);

    /// Lower from PIRTM source (full pipeline).
    LoweringResult lower_from_source(const std::string& source);

private:
    /// Apply PIRTM dialect lowering passes.
    void apply_pirtm_passes(mlir::ModuleOp module);

    /// Apply target-specific lowering.
    void apply_target_lowering(mlir::ModuleOp module);

    /// Verify the lowered module.
    bool verify(mlir::ModuleOp module);

    /// Get current timestamp.
    uint64_t now_ns() const;

    LoweringConfig config_;
};

} // namespace pirtm

#endif // PIRTM_LOWERING_H
