//===- PirtmDialect.h - PIRTM Dialect Declaration -*- C++ -*-===//
//
// Declares the PIRTM MLIR dialect: types, operations, and verification.
//
//===----------------------------------------------------------------------===//

#ifndef PIRTM_DIALECT_H
#define PIRTM_DIALECT_H

#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/OpImplementation.h"

namespace mlir {
namespace pirtm {

/// Get the PIRTM dialect namespace.
StringRef getPirtmDialectNamespace();

} // namespace pirtm
} // namespace mlir

#define GET_TYPEDEF_CLASSES
#include "pirtm/Dialect/PirtmOpsTypes.h.inc"

#define GET_OP_CLASSES
#include "pirtm/Dialect/PirtmOps.h.inc"

#endif // PIRTM_DIALECT_H
