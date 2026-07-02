//===- AdmissibilityValidator.h - AST Validation -*- C++ -*-===//
//
// Validates AST expressions before compilation.
// Non-recursive. Every rejection produces a receipt explaining why.
//
//===----------------------------------------------------------------------===//

#ifndef ADMISSIBILITY_VALIDATOR_H
#define ADMISSIBILITY_VALIDATOR_H

#include <cstdint>
#include <string>
#include <vector>
#include <variant>

namespace pirtm {

/// AST node types (simplified representation).
enum class ExprKind {
    Literal,
    Ident,
    Atom,
    Binary,
    Call,
    If,
    Successor,
    StratumBoundary
};

/// A simplified AST expression for validation.
struct ExprNode {
    ExprKind kind;

    // Literal value
    uint64_t literal_value;

    // Identifier name
    std::string ident_name;

    // Atom prime index
    uint64_t prime_index;

    // Binary operation
    std::string binary_op;
    std::vector<ExprNode> binary_operands;

    // Function call
    std::string call_name;
    std::vector<ExprNode> call_args;

    // If condition
    ExprNode* if_cond;
    std::vector<ExprNode> then_branch;
    std::vector<ExprNode> else_branch;

    // Successor/StratumBoundary inner
    ExprNode* inner;

    ExprNode() : kind(ExprKind::Literal), literal_value(0), prime_index(0),
                 if_cond(nullptr), inner(nullptr) {}
};

/// Validation error with receipt.
struct ValidationError {
    std::string code;       // e.g., "ADM001"
    std::string message;    // Human-readable description
    std::string receipt_hash; // SHA-256 of the rejection

    std::string to_string() const;
};

/// Validation result.
struct ValidationResult {
    bool ok;
    std::vector<ValidationError> errors;

    ValidationResult() : ok(true) {}

    void add_error(const std::string& code, const std::string& message);
};

/// Validator for PIRTM AST expressions.
class AdmissibilityValidator {
public:
    AdmissibilityValidator();

    /// Validate an expression tree.
    ValidationResult validate(const ExprNode& expr);

    /// Validate a program (list of statements).
    ValidationResult validate_program(const std::vector<ExprNode>& stmts);

private:
    /// Validate a single expression.
    void validate_expr(const ExprNode& expr, ValidationResult& result);

    /// Check prime index validity.
    bool check_prime_index(uint64_t prime, ValidationResult& result);

    /// Check binary operation validity.
    bool check_binary_op(const std::string& op, const std::vector<ExprNode>& operands,
                         ValidationResult& result);

    /// Check function call validity.
    bool check_call(const std::string& name, const std::vector<ExprNode>& args,
                    ValidationResult& result);

    /// Check stratum boundary validity.
    bool check_stratum_boundary(const ExprNode& inner, ValidationResult& result);

    /// Check successor bounds.
    bool check_successor_bounds(uint64_t value, ValidationResult& result);

    /// Generate a rejection receipt hash.
    std::string rejection_hash(const std::string& code, const ExprNode& expr);

    uint64_t counter_;
};

} // namespace pirtm

#endif // ADMISSIBILITY_VALIDATOR_H
