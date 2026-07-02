# sovereign-pirtm

> C++ compiler core for the SnapKitty ecosystem.
> MLIR dialect, multiplicity functor, contractivity receipts, sedona spine, zeno-finton control, lean FFI, LLVM codegen.

[![License: Sovereign Source](https://img.shields.io/badge/License-Sovereign%20Source-blue.svg)](SOVEREIGN.md)
[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
[![LLVM](https://img.shields.io/badge/LLVM-17-orange.svg)](https://llvm.org/)

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    SOVEREIGN PIRTM COMPILER                      │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │                    Source Language                        │    │
│  │              (SnapKitty / PIRTM DSL)                     │    │
│  └─────────────────────────────────────────────────────────┘    │
│                           │                                      │
│                           ▼                                      │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │                   Lexer / Parser                         │    │
│  │              (Antlr4 / hand-written)                     │    │
│  └─────────────────────────────────────────────────────────┘    │
│                           │                                      │
│                           ▼                                      │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │              PIRTM MLIR Dialect                          │    │
│  │                                                         │    │
│  │  operator_atom  binary_add  binary_sub  binary_mul     │    │
│  │  binary_div     constant    yield        return         │    │
│  │  stratum_boundary  successor                            │    │
│  └─────────────────────────────────────────────────────────┘    │
│                           │                                      │
│          ┌────────────────┼────────────────┐                    │
│          ▼                ▼                ▼                    │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────────┐       │
│  │ Multiplicity │ │Contractivity │ │  Admissibility   │       │
│  │   Functor    │ │   Receipts   │ │   Validator      │       │
│  │   (p^m, Q)  │ │  (SHA-256)   │ │  (staged)        │       │
│  └──────────────┘ └──────────────┘ └──────────────────┘       │
│          │                │                │                    │
│          └────────────────┼────────────────┘                    │
│                           ▼                                      │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │                  Sedona Spine                             │    │
│  │              (FFI Closure Enforcement)                   │    │
│  │         Single-crossing / RAII guards / Phase tags      │    │
│  └─────────────────────────────────────────────────────────┘    │
│                           │                                      │
│                           ▼                                      │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │               Zeno-Finton Control                         │    │
│  │          κ(t) = κ₀ · e^(-αt)                            │    │
│  │        Exponential decay gain function                   │    │
│  └─────────────────────────────────────────────────────────┘    │
│                           │                                      │
│                           ▼                                      │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │                  Lean FFI Bridge                          │    │
│  │            (Proof verification)                          │    │
│  └─────────────────────────────────────────────────────────┘    │
│                           │                                      │
│                           ▼                                      │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │               LLVM / WASM Codegen                        │    │
│  │          (MLIR → LLVM IR → WebAssembly)                  │    │
│  └─────────────────────────────────────────────────────────┘    │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

## Module Overview

| Module | Directory | Description |
|--------|-----------|-------------|
| **pirtm-mlir** | `pirtm-mlir/` | Custom MLIR dialect for PIRTM operations |
| **multiplicity** | `multiplicity/` | Rational exponentiation: `p^m` where `m ∈ Q` |
| **contractivity** | `contractivity/` | SHA-256 cryptographic receipts, Merkle chain |
| **sedona-spine** | `sedona-spine/` | FFI closure enforcement: single-crossing |
| **zeno-finton** | `zeno-finton/` | Exponential decay gain: `κ(t) = κ₀ · e^(-αt)` |
| **admissibility** | `admissibility/` | AST validation, rejection receipts |
| **lean-ffi** | `lean-ffi/` | Lean 4 proof verification bridge |
| **pirtm-llvm** | `pirtm-llvm/` | MLIR → LLVM IR / WebAssembly lowering |

## MLIR Dialect

```mlir
// Example PIRTM MLIR
func.func @main() {
  // Operator atom
  %a = pirtm.operator_atom "add" : i32
  
  // Binary operations
  %b = pirtm.binary_add %a, %c : i32
  %d = pirtm.binary_mul %b, %e : i32
  
  // Stratum boundary (non-recursive)
  pirtm.stratum_boundary 1
  
  // Constant
  %f = pirtm.constant 42 : i32
  
  // Return
  pirtm.return %f : i32
}
```

## Multiplicity Functor

```
p^m where m ∈ Q (Rational64)

Examples:
  2^(3/1) = 8      (integer exponent)
  2^(1/2) = 1.414  (square root)
  2^(1/3) = 1.260  (cube root)
  2^(-1) = 0.5     (negative exponent)
```

## Contractivity Receipt

```json
{
  "standard": "PIRTM-CONTRACTIVITY-1",
  "algorithm": "sha256-merkle-v1",
  "input_hash": "sha256:a1b2c3d4...",
  "output_hash": "sha256:e5f6a7b8...",
  "receipt_hash": "sha256:c9d0e1f2...",
  "status": "contractive",
  "timestamp": "2025-01-01T00:00:00Z"
}
```

## Build

### Prerequisites

- LLVM 17+
- CMake 3.20+
- Clang 17+
- Lean 4 (optional, for proof verification)

### Build

```bash
mkdir build && cd build
cmake .. -DLLVM_DIR=/path/to/llvm/lib/cmake/llvm
make -j$(nproc)
```

### Test

```bash
# Run all tests
ctest --output-on-failure

# Run specific module tests
./build/multiplicity_test
./build/contractivity_test
./build/admissibility_test
```

## Usage

```cpp
#include "pirtm-mlir/PIRTMDialect.h"
#include "multiplicity/Multiplicity.h"
#include "contractivity/ContractivityReceipt.h"
#include "zeno-finton/ZenoFinton.h"

using namespace pirtm;

int main() {
  // Multiplicity functor
  Multiplicity m(2, Rational64(3, 1));
  auto result = m.compute();  // 8
  
  // Contractivity receipt
  ContractivityReceipt receipt;
  auto seal = receipt.seal(input, output);
  
  // Zeno-Finton control
  ZenoFinton zf(1.0, 0.1);  // κ₀=1.0, α=0.1
  double gain = zf.gain(10.0);  // κ(10) = 1.0 * e^(-1.0) ≈ 0.368
  
  return 0;
}
```

## Invariants

| Invariant | Description |
|-----------|-------------|
| **No Recursion** | MLIR dialect has no recursive ops |
| **Type Safe** | All operations are typed at MLIR level |
| **Deterministic** | Same input → same LLVM IR |
| **WORM Sealed** | Contractivity receipts are write-once |
| **Phase Separated** | FFI calls cross single phase boundary |

## License

Sovereign Source License — see [SOVEREIGN.md](SOVEREIGN.md)

---

```
SOVEREIGN-PIRTM-001
Dialect. Contract. Decay. Verify. Emit.
Same source. Same IR.
No recursion. No borrowed thesis.
```
