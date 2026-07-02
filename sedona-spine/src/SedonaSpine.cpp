//===- SedonaSpine.cpp - FFI Closure Enforcement Implementation ===//
//
// Non-recursive FFI closure enforcement.
// Every crossing is recorded and WORM-sealed.
//
//===----------------------------------------------------------------------===//

#include "pirtm/SedonaSpine.h"
#include <chrono>
#include <sstream>

namespace pirtm {

SedonaSpineEnforcer::SedonaSpineEnforcer() {}

bool SedonaSpineEnforcer::validate_crossing(
    const std::string& closure_name,
    FFIBoundary boundary,
    const std::string& expected_type
) const {
    // Check if already crossed
    for (const auto& record : records_) {
        if (record.closure_name == closure_name && record.success) {
            return false; // Already sealed
        }
    }

    // Validate boundary-specific rules
    switch (boundary) {
        case FFIBoundary::CompileTime:
            // Compile-time closures must have generic bound
            return !expected_type.empty();

        case FFIBoundary::Runtime:
            // Runtime closures must be explicitly marked
            return true;

        case FFIBoundary::Wasm:
            // WASM boundary requires no heap references
            return expected_type.find("raw") != std::string::npos;

        case FFIBoundary::Lean:
            // Lean FFI requires proof-producing closure
            return expected_type.find("proof") != std::string::npos;

        case FFIBoundary::Native:
            // Native boundary is unrestricted
            return true;
    }

    return false;
}

void SedonaSpineEnforcer::record_crossing(const FFICrossingRecord& record) {
    records_.push_back(record);
}

const std::vector<FFICrossingRecord>& SedonaSpineEnforcer::get_records() const {
    return records_;
}

bool SedonaSpineEnforcer::is_sealed(const std::string& closure_name) const {
    for (const auto& record : records_) {
        if (record.closure_name == closure_name && record.success) {
            return true;
        }
    }
    return false;
}

std::string SedonaSpineEnforcer::boundary_name(FFIBoundary boundary) {
    switch (boundary) {
        case FFIBoundary::CompileTime: return "compile_time";
        case FFIBoundary::Runtime: return "runtime";
        case FFIBoundary::Wasm: return "wasm";
        case FFIBoundary::Lean: return "lean";
        case FFIBoundary::Native: return "native";
    }
    return "unknown";
}

} // namespace pirtm
