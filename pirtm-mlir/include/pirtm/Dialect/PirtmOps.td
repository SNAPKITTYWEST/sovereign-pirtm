//===- PirtmOps.td - PIRTM Dialect Operations -*- tablegen -*-===//
//
// Defines the PIRTM MLIR dialect operations for prime-indexed operator atoms.
// Non-recursive. Every op has typed I/O and emits a deterministic WORM seal.
//
//===----------------------------------------------------------------------===//

#ifndef PIRTM_OPS_TD
#define PIRTM_OPS_TD

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

def Pirtm_StratumType : TypeDef<Pirtm_Dialect, "Stratum"> {
  let summary = "Stratum type for prime-indexed operator atoms";
  let description = [{
    Represents a value in the PIRTM stratum lattice.
    Every Stratum carries a prime index and a contractivity receipt hash.
  }];
}

def Pirtm_PrimeAttr : AttrDef<Pirtm_Dialect, "PrimeAttr"> {
  let summary = "Prime index attribute";
  let parameters = (ins "uint64_t":$value);
  let assemblyFormat = "`prime_index` `=` $value";
}

def Pirtm_ReceiptHashAttr : AttrDef<Pirtm_Dialect, "ReceiptHashAttr"> {
  let summary = "Contractivity receipt hash attribute";
  let parameters = (ins "StringAttr":$hash);
  let assemblyFormat = "`receipt` `=` $hash";
}

// --- Operations ---

def Pirtm_OperatorAtomOp : Op<Pirtm_Dialect, "operator_atom"> {
  let summary = "Prime-indexed operator atom";
  let description = [{
    The fundamental PIRTM operation. Creates a stratum value from a prime index.
    Requires a valid contractivity receipt at construction time.
  }];

  let arguments = (ins
    "IndexAttr": $primeIndex,
    "StringAttr": $receiptHash
  );

  let results = (outs Pirtm_StratumType: $result);

  let assemblyFormat = [{
    `(` $primeIndex `,` $receiptHash `)` attr-dict `:` type($result)
  }];

  let hasVerifier = 1;
}

def Pirtm_BinaryAddOp : Op<Pirtm_Dialect, "binary_add"> {
  let summary = "Stratum addition (multiplicity combine)";
  let description = [{
    Combines two stratum values via multiplicity functor.
    Result prime = left^right (rational exponentiation).
  }];

  let arguments = (ins
    Pirtm_StratumType: $lhs,
    Pirtm_StratumType: $rhs,
    "StringAttr": $receiptHash
  );

  let results = (outs Pirtm_StratumType: $result);

  let assemblyFormat = [{
    `(` $lhs `,` $rhs `,` $receiptHash `)` attr-dict `:` type($result)
  }];

  let hasVerifier = 1;
}

def Pirtm_BinarySubOp : Op<Pirtm_Dialect, "binary_sub"> {
  let summary = "Stratum subtraction";

  let arguments = (ins
    Pirtm_StratumType: $lhs,
    Pirtm_StratumType: $rhs,
    "StringAttr": $receiptHash
  );

  let results = (outs Pirtm_StratumType: $result);

  let assemblyFormat = [{
    `(` $lhs `,` $rhs `,` $receiptHash `)` attr-dict `:` type($result)
  }];
}

def Pirtm_BinaryMulOp : Op<Pirtm_Dialect, "binary_mul"> {
  let summary = "Stratum multiplication";

  let arguments = (ins
    Pirtm_StratumType: $lhs,
    Pirtm_StratumType: $rhs,
    "StringAttr": $receiptHash
  );

  let results = (outs Pirtm_StratumType: $result);

  let assemblyFormat = [{
    `(` $lhs `,` $rhs `,` $receiptHash `)` attr-dict `:` type($result)
  }];
}

def Pirtm_BinaryDivOp : Op<Pirtm_Dialect, "binary_div"> {
  let summary = "Stratum division";

  let arguments = (ins
    Pirtm_StratumType: $lhs,
    Pirtm_StratumType: $rhs,
    "StringAttr": $receiptHash
  );

  let results = (outs Pirtm_StratumType: $result);

  let assemblyFormat = [{
    `(` $lhs `,` $rhs `,` $receiptHash `)` attr-dict `:` type($result)
  }];
}

def Pirtm_ConstantOp : Op<Pirtm_Dialect, "constant"> {
  let summary = "Integer constant";

  let arguments = (ins "IntegerAttr": $value);
  let results = (outs Pirtm_StratumType: $result);

  let assemblyFormat = [{
    `(` $value `)` attr-dict `:` type($result)
  }];
}

def Pirtm_StratumBoundaryOp : Op<Pirtm_Dialect, "stratum_boundary"> {
  let summary = "Stratum boundary marker";
  let description = [{
    Explicitly models the stratum boundary for Sedona Spine.
    Enforces that the inner expression does not cross strata illegally.
  }];

  let arguments = (ins
    Pirtm_StratumType: $input,
    "StringAttr": $receiptHash
  );

  let results = (outs Pirtm_StratumType: $result);

  let assemblyFormat = [{
    `(` $input `,` $receiptHash `)` attr-dict `:` type($result)
  }];

  let hasVerifier = 1;
}

def Pirtm_SuccessorOp : Op<Pirtm_Dialect, "successor"> {
  let summary = "Successor operation";
  let description = [{
    Computes the successor of a stratum value.
    Enforces bounds checking (value < i64::MAX).
  }];

  let arguments = (ins
    Pirtm_StratumType: $input,
    "StringAttr": $receiptHash
  );

  let results = (outs Pirtm_StratumType: $result);

  let assemblyFormat = [{
    `(` $input `,` $receiptHash `)` attr-dict `:` type($result)
  }];

  let hasVerifier = 1;
}

def Pirtm_YieldOp : Op<Pirtm_Dialect, "yield"> {
  let summary = "Block terminator yield";
  let description = [{ Yields a stratum value as the block result. }];

  let arguments = (ins Pirtm_StratumType: $value);
  let assemblyFormat = [{ `(` $value `)` attr-dict `:` type($value) }];

  let terminator = 1;
}

def Pirtm_ReturnOp : Op<Pirtm_Dialect, "return"> {
  let summary = "Function return";
  let description = [{ Returns a stratum value from the function. }];

  let arguments = (ins Pirtm_StratumType: $value);
  let assemblyFormat = [{ `(` $value `)` attr-dict `:` type($value) }];

  let terminator = 1;
}

#endif // PIRTM_OPS_TD
