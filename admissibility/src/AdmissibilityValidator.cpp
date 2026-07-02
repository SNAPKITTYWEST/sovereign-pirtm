//===- AdmissibilityValidator.cpp - AST Validation Implementation ===//
//
// Non-recursive AST validation.
// Every rejection produces a receipt explaining why.
//
//===----------------------------------------------------------------------===//

#include "pirtm/AdmissibilityValidator.h"
#include "pirtm/ContractivityReceipt.h"
#include <sstream>
#include <algorithm>

namespace pirtm {

// --- ValidationError ---

std::string ValidationError::to_string() const {
    return "[" + code + "] " + message + " (receipt: " + receipt_hash + ")";
}

// --- ValidationResult ---

void ValidationResult::add_error(const std::string& code, const std::string& message) {
    ok = false;
    ValidationError err;
    err.code = code;
    err.message = message;
    err.receipt_hash = ""; // Will be set by validator
    errors.push_back(err);
}

// --- AdmissibilityValidator ---

AdmissibilityValidator::AdmissibilityValidator() : counter_(0) {}

ValidationResult AdmissibilityValidator::validate(const ExprNode& expr) {
    ValidationResult result;
    validate_expr(expr, result);
    return result;
}

ValidationResult AdmissibilityValidator::validate_program(const std::vector<ExprNode>& stmts) {
    ValidationResult result;
    for (const auto& stmt : stmts) {
        validate_expr(stmt, result);
        if (!result.ok) break; // Fail-fast on first error
    }
    return result;
}

void AdmissibilityValidator::validate_expr(const ExprNode& expr, ValidationResult& result) {
    switch (expr.kind) {
        case ExprKind::Literal:
            // Literals are always valid
            break;

        case ExprKind::Ident:
            // Identifiers are always valid (resolved later)
            break;

        case ExprKind::Atom:
            check_prime_index(expr.prime_index, result);
            break;

        case ExprKind::Binary:
            check_binary_op(expr.binary_op, expr.binary_operands, result);
            break;

        case ExprKind::Call:
            check_call(expr.call_name, expr.call_args, result);
            break;

        case ExprKind::If:
            if (expr.if_cond) validate_expr(*expr.if_cond, result);
            for (const auto& s : expr.then_branch) validate_expr(s, result);
            for (const auto& s : expr.else_branch) validate_expr(s, result);
            break;

        case ExprKind::Successor:
            if (expr.inner) {
                validate_expr(*expr.inner, result);
                if (result.ok && expr.inner->kind == ExprKind::Literal) {
                    check_successor_bounds(expr.inner->literal_value, result);
                }
            }
            break;

        case ExprKind::StratumBoundary:
            if (expr.inner) {
                validate_expr(*expr.inner, result);
                if (result.ok) {
                    check_stratum_boundary(*expr.inner, result);
                }
            }
            break;
    }
}

bool AdmissibilityValidator::check_prime_index(uint64_t prime, ValidationResult& result) {
    if (prime < 2) {
        std::ostringstream oss;
        oss << "ADM001: Invalid prime index " << prime << " (must be >= 2)";
        result.add_error("ADM001", oss.str());
        return false;
    }
    return true;
}

bool AdmissibilityValidator::check_binary_op(const std::string& op,
                                              const std::vector<ExprNode>& operands,
                                              ValidationResult& result) {
    // Validate operator
    static const std::vector<std::string> valid_ops = {"add", "sub", "mul", "div"};
    if (std::find(valid_ops.begin(), valid_ops.end(), op) == valid_ops.end()) {
        result.add_error("ADM002", "Unknown binary operator: " + op);
        return false;
    }

    // Validate operands
    if (operands.size() != 2) {
        result.add_error("ADM003", "Binary operator requires exactly 2 operands");
        return false;
    }

    // Validate operand types for division
    if (op == "div" && operands[1].kind == ExprKind::Literal && operands[1].literal_value == 0) {
        result.add_error("ADM004", "Division by zero");
        return false;
    }

    return true;
}

bool AdmissibilityValidator::check_call(const std::string& name,
                                         const std::vector<ExprNode>& args,
                                         ValidationResult& result) {
    // Validate function name
    static const std::vector<std::string> valid_funcs = {"Ap", "succ", "stratum_boundary"};
    if (std::find(valid_funcs.begin(), valid_funcs.end(), name) == valid_funcs.end()) {
        result.add_error("ADM005", "Unknown function: " + name);
        return false;
    }

    // Validate argument count
    if (name == "Ap" && args.size() != 1) {
        result.add_error("ADM006", "Ap() requires exactly 1 argument");
        return false;
    }

    return true;
}

bool AdmissibilityValidator::check_stratum_boundary(const ExprNode& inner,
                                                     ValidationResult& result) {
    // Boundary cannot be zero
    if (inner.kind == ExprKind::Literal && inner.literal_value == 0) {
        result.add_error("ADM007", "Invalid boundary zero in StratumBoundary");
        return false;
    }
    return true;
}

bool AdmissibilityValidator::check_successor_bounds(uint64_t value, ValidationResult& result) {
    // Check overflow (i64::MAX as upper bound)
    const uint64_t i64_max = static_cast<uint64_t>(9223372036854775807LL);
    if (value > i64_max) {
        std::ostringstream oss;
        oss << "ADM008: Successor bounds check violation: " << value << " > i64::MAX";
        result.add_error("ADM008", oss.str());
        return false;
    }
    return true;
}

std::string AdmissibilityValidator::rejection_hash(const std::string& code,
                                                    const ExprNode& expr) {
    std::ostringstream data;
    data << "REJECT:" << code << ":counter=" << counter_++;
    return sha256_hex(data.str());
}

} // namespace pirtm
