//===- ZenoFinton.h - Exponential Decay Gain Controller -*- C++ -*-===//
//
// Zeno-Finton controller for runtime drift detection.
// Applies exponential decay gain: κ(t) = κ₀ · e^(-αt)
// Non-recursive. Deterministic output.
//
//===----------------------------------------------------------------------===//

#ifndef ZENO_FINTON_H
#define ZENO_FINTON_H

#include <cstdint>
#include <string>
#include <chrono>

namespace pirtm {

/// Configuration for the Zeno-Finton controller.
struct ZenoConfig {
    double kappa0;      // Initial gain (default: 1.0)
    double alpha;       // Decay rate (default: 0.1)
    double rho_warn;    // Warning threshold (default: 0.85)
    double rho_halt;    // Halt threshold (default: 1.0)
    double delta_max;   // Max drift threshold (default: 1e-4)
    uint64_t poll_ms;   // Poll interval in milliseconds (default: 100)

    ZenoConfig();
    static ZenoConfig defaults();
};

/// Manifold state for drift detection.
struct ManifoldState {
    double rho;             // Manifold drift metric
    double delta;           // Liquidity pool drift
    double lambda_l_product; // Stability product (λ·L)
    uint64_t timestamp_ns;  // Timestamp in nanoseconds

    ManifoldState();
};

/// Actions the controller can request.
enum class ControllerAction {
    Continue,       // Normal operation
    Warning,        // Warning threshold exceeded
    Kill,           // Halt threshold exceeded - kill switch
    Error           // Error state
};

/// Result of a controller step.
struct ControllerResult {
    ControllerAction action;
    double raw_rho;
    double attenuated_rho;
    double kappa;
    std::string message;

    ControllerResult();
};

/// Zeno-Finton exponential decay gain controller.
class ZenoFintonController {
public:
    explicit ZenoFintonController(const ZenoConfig& config = ZenoConfig::defaults());

    /// Compute current κ(t) = κ₀ · e^(-αt).
    double compute_kappa() const;

    /// Apply gain to raw rho: attenuated = raw · (1 - κ).
    double apply_gain(double raw_rho) const;

    /// Process a manifold state and return action.
    ControllerResult step(const ManifoldState& state);

    /// Get elapsed time since controller creation.
    double elapsed_seconds() const;

    /// Reset the controller (new epoch).
    void reset();

    /// Get current config.
    const ZenoConfig& config() const { return config_; }

private:
    ZenoConfig config_;
    std::chrono::steady_clock::time_point start_time_;
};

/// Format a controller action as a human-readable string.
std::string format_action(ControllerAction action);

/// Format a controller result as JSON.
std::string result_to_json(const ControllerResult& result);

} // namespace pirtm

#endif // ZENO_FINTON_H
