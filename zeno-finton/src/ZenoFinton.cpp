//===- ZenoFinton.cpp - Exponential Decay Controller Implementation ===//
//
// Non-recursive Zeno-Finton controller.
// κ(t) = κ₀ · e^(-αt) provides exponential decay of gain.
//
//===----------------------------------------------------------------------===//

#include "pirtm/ZenoFinton.h"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace pirtm {

// --- ZenoConfig ---

ZenoConfig::ZenoConfig()
    : kappa0(1.0), alpha(0.1), rho_warn(0.85), rho_halt(1.0),
      delta_max(1e-4), poll_ms(100) {}

ZenoConfig ZenoConfig::defaults() {
    return ZenoConfig();
}

// --- ManifoldState ---

ManifoldState::ManifoldState()
    : rho(0.0), delta(0.0), lambda_l_product(0.0), timestamp_ns(0) {}

// --- ControllerResult ---

ControllerResult::ControllerResult()
    : action(ControllerAction::Continue), raw_rho(0.0), attenuated_rho(0.0),
      kappa(0.0), message("") {}

// --- ZenoFintonController ---

ZenoFintonController::ZenoFintonController(const ZenoConfig& config)
    : config_(config), start_time_(std::chrono::steady_clock::now()) {}

double ZenoFintonController::compute_kappa() const {
    double t = elapsed_seconds();
    return config_.kappa0 * std::exp(-config_.alpha * t);
}

double ZenoFintonController::apply_gain(double raw_rho) const {
    double kappa = compute_kappa();
    double attenuated = raw_rho * (1.0 - kappa);
    return std::max(0.0, attenuated);
}

ControllerResult ZenoFintonController::step(const ManifoldState& state) {
    ControllerResult result;
    result.raw_rho = state.rho;
    result.kappa = compute_kappa();

    // Check λ·L stability product
    if (state.lambda_l_product >= 1.0) {
        result.action = ControllerAction::Kill;
        result.attenuated_rho = state.rho;
        std::ostringstream oss;
        oss << "Finton stability violation: λL = " << std::fixed << std::setprecision(6)
            << state.lambda_l_product << " >= 1.0";
        result.message = oss.str();
        return result;
    }

    // Check delta drift
    if (state.delta >= config_.delta_max) {
        result.action = ControllerAction::Kill;
        result.attenuated_rho = state.rho;
        std::ostringstream oss;
        oss << "Liquidity pool drift exceeded: δ = " << std::fixed << std::setprecision(6)
            << state.delta << " >= " << config_.delta_max;
        result.message = oss.str();
        return result;
    }

    // Apply Zeno-Finton gain
    double attenuated = apply_gain(state.rho);
    result.attenuated_rho = attenuated;

    // Check halt threshold
    if (attenuated >= config_.rho_halt) {
        result.action = ControllerAction::Kill;
        std::ostringstream oss;
        oss << "Halt threshold exceeded: ρ = " << std::fixed << std::setprecision(6)
            << attenuated << " >= " << config_.rho_halt;
        result.message = oss.str();
        return result;
    }

    // Check warning threshold
    if (attenuated >= config_.rho_warn) {
        result.action = ControllerAction::Warning;
        std::ostringstream oss;
        oss << "Drift warning: ρ = " << std::fixed << std::setprecision(6)
            << attenuated << " >= " << config_.rho_warn;
        result.message = oss.str();
        return result;
    }

    // Normal operation
    result.action = ControllerAction::Continue;
    result.message = "Drift within bounds";
    return result;
}

double ZenoFintonController::elapsed_seconds() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(now - start_time_).count();
}

void ZenoFintonController::reset() {
    start_time_ = std::chrono::steady_clock::now();
}

// --- Formatting ---

std::string format_action(ControllerAction action) {
    switch (action) {
        case ControllerAction::Continue: return "CONTINUE";
        case ControllerAction::Warning: return "WARNING";
        case ControllerAction::Kill: return "KILL";
        case ControllerAction::Error: return "ERROR";
    }
    return "UNKNOWN";
}

std::string result_to_json(const ControllerResult& result) {
    std::ostringstream oss;
    oss << "{\"action\":\"" << format_action(result.action) << "\""
        << ",\"raw_rho\":" << std::fixed << std::setprecision(6) << result.raw_rho
        << ",\"attenuated_rho\":" << result.attenuated_rho
        << ",\"kappa\":" << result.kappa
        << ",\"message\":\"" << result.message << "\"}";
    return oss.str();
}

} // namespace pirtm
