//===- SedonaSpine.h - FFI Closure Enforcement -*- C++ -*-===//
//
// Enforces closure rules before crossing FFI boundaries.
// Non-recursive. Every crossing produces a WORM-sealed receipt.
//
//===----------------------------------------------------------------------===//

#ifndef SEDONA_SPINE_H
#define SEDONA_SPINE_H

#include <cstdint>
#include <string>
#include <functional>
#include <vector>
#include <optional>

namespace pirtm {

/// FFI closure boundary types.
enum class FFIBoundary {
    CompileTime,    // Compile-time generic bound
    Runtime,        // Runtime closure execution
    Wasm,           // WASM boundary crossing
    Lean,           // Lean FFI boundary
    Native          // Native code boundary
};

/// A sealed closure that can only cross FFI once.
template<typename Input, typename Output>
class SealedClosure {
public:
    using ClosureFn = std::function<Output(const Input&)>;

    SealedClosure(ClosureFn fn, const std::string& name)
        : fn_(std::move(fn)), name_(name), crossed_(false) {}

    /// Execute the closure, enforcing single-crossing rule.
    std::optional<Output> execute(const Input& input, FFIBoundary boundary) {
        if (crossed_) {
            return std::nullopt; // Already crossed - sealed
        }
        crossed_ = true;
        last_boundary_ = boundary;
        return fn_(input);
    }

    bool has_crossed() const { return crossed_; }
    FFIBoundary last_boundary() const { return last_boundary_; }
    const std::string& name() const { return name_; }

private:
    ClosureFn fn_;
    std::string name_;
    bool crossed_;
    FFIBoundary last_boundary_;
};

/// Record of an FFI crossing.
struct FFICrossingRecord {
    std::string closure_name;
    FFIBoundary boundary;
    uint64_t timestamp_ns;
    bool success;
    std::string error_msg;
    std::string receipt_hash;
};

/// Enforcer for FFI closure rules.
class SedonaSpineEnforcer {
public:
    SedonaSpineEnforcer();

    /// Validate that a closure can cross the given boundary.
    /// Returns true if crossing is allowed.
    bool validate_crossing(
        const std::string& closure_name,
        FFIBoundary boundary,
        const std::string& expected_type
    ) const;

    /// Record a crossing event.
    void record_crossing(const FFICrossingRecord& record);

    /// Get all crossing records.
    const std::vector<FFICrossingRecord>& get_records() const;

    /// Check if a closure has already crossed (is sealed).
    bool is_sealed(const std::string& closure_name) const;

    /// Get the boundary type name as string.
    static std::string boundary_name(FFIBoundary boundary);

private:
    std::vector<FFICrossingRecord> records_;
};

/// RAII guard for FFI boundary crossing.
template<typename Input, typename Output>
class FFIGuard {
public:
    FFIGuard(SealedClosure<Input, Output>& closure, SedonaSpineEnforcer& enforcer,
             FFIBoundary boundary)
        : closure_(closure), enforcer_(enforcer), boundary_(boundary) {}

    ~FIGuard() {
        // Record the crossing on destruction
        FFICrossingRecord record;
        record.closure_name = closure_.name();
        record.boundary = boundary_;
        record.timestamp_ns = get_timestamp();
        record.success = !closure_.has_crossed(); // Sealed before this crossing
        enforcer_.record_crossing(record);
    }

    /// Execute the closure through the guard.
    std::optional<Output> execute(const Input& input) {
        return closure_.execute(input, boundary_);
    }

private:
    SealedClosure<Input, Output>& closure_;
    SedonaSpineEnforcer& enforcer_;
    FFIBoundary boundary_;

    uint64_t get_timestamp() const {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()).count();
    }
};

} // namespace pirtm

#endif // SEDONA_SPINE_H
