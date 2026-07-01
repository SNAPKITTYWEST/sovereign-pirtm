//! # sovereign-pirtm
//!
//! PIRTM compiler IR: Prime-Indexed Recursive Tensor Mathematics.
//! Non-recursive circuit lowering.

pub mod core {
//! # utqc-core
//!
//! Circuit IR — Gate, Qubit, Circuit, Measurement.
//! Non-recursive. Every circuit compiles to a flat list of operations.

use serde::{Deserialize, Serialize};
use thiserror::Error;

/// Errors in circuit construction or execution.
#[derive(Error, Debug, Clone, PartialEq, Eq)]
pub enum CircuitError {
    /// Qubit index out of bounds.
    #[error("qubit index {0} out of bounds (circuit has {1} qubits)")]
    QubitOutOfBounds(usize, usize),

    /// Duplicate measurement on the same qubit.
    #[error("duplicate measurement on qubit {0}")]
    DuplicateMeasurement(usize),

    /// Empty circuit.
    #[error("circuit is empty")]
    EmptyCircuit,
}

/// A single qubit identifier.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct Qubit(pub usize);

/// Single-qubit gate types.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum SingleGate {
    /// Pauli-X (NOT).
    PauliX,
    /// Pauli-Y.
    PauliY,
    /// Pauli-Z.
    PauliZ,
    /// Hadamard.
    Hadamard,
    /// T-gate (π/8 phase).
    TGate,
    /// S-gate (π/4 phase).
    SGate,
}

/// Two-qubit gate types.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum DoubleGate {
    /// Controlled-NOT.
    CNOT,
    /// Controlled-Z.
    CZ,
    /// SWAP.
    SWAP,
}

/// A gate operation in the circuit.
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub enum Gate {
    /// Single-qubit gate.
    Single {
        /// Gate type.
        gate: SingleGate,
        /// Target qubit.
        target: Qubit,
    },
    /// Two-qubit gate.
    Double {
        /// Gate type.
        gate: DoubleGate,
        /// Control qubit.
        control: Qubit,
        /// Target qubit.
        target: Qubit,
    },
    /// Rotation gate (parameterized).
    Rotation {
        /// Target qubit.
        target: Qubit,
        /// Angle in radians.
        angle: f64,
    },
}

/// A measurement record.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct Measurement {
    /// Qubit being measured.
    pub qubit: Qubit,
    /// Classical bit index to store result.
    pub classical_bit: usize,
}

/// A quantum circuit — non-recursive flat IR.
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct Circuit {
    /// Number of qubits in the circuit.
    pub num_qubits: usize,
    /// Number of classical bits.
    pub num_classical_bits: usize,
    /// Ordered list of gate operations.
    pub gates: Vec<Gate>,
    /// Measurements to perform at the end.
    pub measurements: Vec<Measurement>,
}

impl Circuit {
    /// Create a new empty circuit.
    pub fn new(num_qubits: usize, num_classical_bits: usize) -> Self {
        Self {
            num_qubits,
            num_classical_bits,
            gates: Vec::new(),
            measurements: Vec::new(),
        }
    }

    /// Add a gate to the circuit.
    pub fn add_gate(&mut self, gate: Gate) -> Result<(), CircuitError> {
        match &gate {
            Gate::Single { target, .. } => {
                if target.0 >= self.num_qubits {
                    return Err(CircuitError::QubitOutOfBounds(target.0, self.num_qubits));
                }
            }
            Gate::Double { control, target, .. } => {
                if control.0 >= self.num_qubits {
                    return Err(CircuitError::QubitOutOfBounds(control.0, self.num_qubits));
                }
                if target.0 >= self.num_qubits {
                    return Err(CircuitError::QubitOutOfBounds(target.0, self.num_qubits));
                }
            }
            Gate::Rotation { target, .. } => {
                if target.0 >= self.num_qubits {
                    return Err(CircuitError::QubitOutOfBounds(target.0, self.num_qubits));
                }
            }
        }
        self.gates.push(gate);
        Ok(())
    }

    /// Add a measurement.
    pub fn add_measurement(&mut self, qubit: Qubit, classical_bit: usize) -> Result<(), CircuitError> {
        if qubit.0 >= self.num_qubits {
            return Err(CircuitError::QubitOutOfBounds(qubit.0, self.num_qubits));
        }
        if self.measurements.iter().any(|m| m.qubit == qubit) {
            return Err(CircuitError::DuplicateMeasurement(qubit.0));
        }
        self.measurements.push(Measurement { qubit, classical_bit });
        Ok(())
    }

    /// Number of gates in the circuit.
    pub fn depth(&self) -> usize {
        self.gates.len()
    }

    /// Validate the circuit.
    pub fn validate(&self) -> Result<(), CircuitError> {
        if self.gates.is_empty() && self.measurements.is_empty() {
            return Err(CircuitError::EmptyCircuit);
        }
        Ok(())
    }
}

/// The non-recursive pass trait.
pub trait Pass {
    /// Input type for this pass.
    type Input;
    /// Output type for this pass.
    type Output;

    /// Name of this pass.
    fn name(&self) -> &'static str;

    /// Execute the pass.
    fn run(&self, input: Self::Input) -> Result<Self::Output, CircuitError>;
}

}

//! # sovereign-pirtm
//!
//! PIRTM compiler IR — Prime-Indexed Recursive Tensor Mathematics.
//! Non-recursive circuit lowering.

use crate::core::{Circuit, Gate, Pass, CircuitError};
use serde::{Deserialize, Serialize};
use thiserror::Error;

/// PIRTM error.
#[derive(Error, Debug, Clone, PartialEq, Eq)]
pub enum PirtnError {
    /// Invalid tensor shape.
    #[error("invalid tensor shape")]
    InvalidShape,
    /// Qubit index out of bounds.
    #[error("qubit index {0} out of bounds")]
    QubitOutOfBounds(usize),
    /// Empty program.
    #[error("empty program")]
    EmptyProgram,
}

/// Tensor operation IR.
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub enum TensorOp {
    /// Matrix multiply: C = A × B.
    MatMul { a: usize, b: usize, c: usize },
    /// Element-wise add: out = A + B.
    Add { a: usize, b: usize, out: usize },
    /// Tensor contraction over index i.
    Contract { a: usize, b: usize, out: usize, axis: usize },
    /// Permute axes.
    Permute { input: usize, out: usize, axes: Vec<usize> },
    /// Scalar multiply: out = scalar × tensor.
    ScalarMul { tensor: usize, out: usize, scalar: u64 },
    /// Reshape (metadata only, no gate emission).
    Reshape { input: usize, shape: Vec<usize> },
}

/// PIRTM program: a sequence of tensor operations.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PirtnProgram {
    /// Operations.
    pub ops: Vec<TensorOp>,
    /// Number of tensors.
    pub num_tensors: usize,
}

impl PirtnProgram {
    /// Create new empty program.
    pub fn new(num_tensors: usize) -> Self {
        Self { ops: Vec::new(), num_tensors }
    }

    /// Add an operation.
    pub fn add_op(&mut self, op: TensorOp) {
        self.ops.push(op);
    }

    /// Lower to quantum circuit.
    pub fn lower_to_circuit(&self) -> Result<Circuit, PirtnError> {
        use crate::core::{Qubit, SingleGate, DoubleGate};
        let n = self.num_tensors.max(2);
        if self.ops.is_empty() {
            return Err(PirtnError::EmptyProgram);
        }
        let mut circuit = Circuit::new(n, n);
        for op in &self.ops {
            match op {
                TensorOp::MatMul { a, b, c } => {
                    if *a >= n || *b >= n || *c >= n {
                        return Err(PirtnError::QubitOutOfBounds(n));
                    }
                    // MatMul → H on control, CNOT control→target, H on control
                    let _ = circuit.add_gate(Gate::Single { gate: SingleGate::Hadamard, target: Qubit(*a) });
                    let _ = circuit.add_gate(Gate::Double { gate: DoubleGate::CNOT, control: Qubit(*a), target: Qubit(*b) });
                    let _ = circuit.add_gate(Gate::Single { gate: SingleGate::Hadamard, target: Qubit(*a) });
                }
                TensorOp::Add { a, b, out } => {
                    if *a >= n || *b >= n || *out >= n {
                        return Err(PirtnError::QubitOutOfBounds(n));
                    }
                    // Add → CNOT from both inputs to output
                    let _ = circuit.add_gate(Gate::Double { gate: DoubleGate::CNOT, control: Qubit(*a), target: Qubit(*out) });
                    let _ = circuit.add_gate(Gate::Double { gate: DoubleGate::CNOT, control: Qubit(*b), target: Qubit(*out) });
                }
                TensorOp::Contract { a, b, out, axis } => {
                    if *a >= n || *b >= n || *out >= n {
                        return Err(PirtnError::QubitOutOfBounds(n));
                    }
                    // Contraction → Toffoli-like sequence
                    let _ = circuit.add_gate(Gate::Single { gate: SingleGate::Hadamard, target: Qubit(*out) });
                    let _ = circuit.add_gate(Gate::Double { gate: DoubleGate::CNOT, control: Qubit(*a), target: Qubit(*out) });
                    let _ = circuit.add_gate(Gate::Double { gate: DoubleGate::CNOT, control: Qubit(*b), target: Qubit(*out) });
                    let _ = circuit.add_gate(Gate::Single { gate: SingleGate::Hadamard, target: Qubit(*out) });
                    let _ = axis;
                }
                TensorOp::Permute { input, out, axes } => {
                    if *input >= n || *out >= n {
                        return Err(PirtnError::QubitOutOfBounds(n));
                    }
                    // Permute → SWAP chain based on axis permutation
                    for (i, &target) in axes.iter().enumerate() {
                        if i != target && target < n {
                            let _ = circuit.add_gate(Gate::Double { gate: DoubleGate::SWAP, control: Qubit(i), target: Qubit(target) });
                        }
                    }
                    let _ = input;
                    let _ = out;
                }
                TensorOp::ScalarMul { tensor, out, scalar } => {
                    if *tensor >= n || *out >= n {
                        return Err(PirtnError::QubitOutOfBounds(n));
                    }
                    // ScalarMul → repeated controlled rotations
                    if *scalar == 1 {
                        let _ = circuit.add_gate(Gate::Double { gate: DoubleGate::CNOT, control: Qubit(*tensor), target: Qubit(*out) });
                    } else if *scalar == 2 {
                        let _ = circuit.add_gate(Gate::Single { gate: SingleGate::PauliX, target: Qubit(*tensor) });
                        let _ = circuit.add_gate(Gate::Double { gate: DoubleGate::CNOT, control: Qubit(*tensor), target: Qubit(*out) });
                        let _ = circuit.add_gate(Gate::Single { gate: SingleGate::PauliX, target: Qubit(*tensor) });
                    }
                }
                TensorOp::Reshape { .. } => {}
            }
        }
        Ok(circuit)
    }

    /// Count operations.
    pub fn op_count(&self) -> usize {
        self.ops.len()
    }

    /// Count qubits required.
    pub fn qubit_count(&self) -> usize {
        self.num_tensors
    }
}

/// The PIRTM lowering pass.
pub struct PirtnLower;

impl Pass for PirtnLower {
    type Input = PirtnProgram;
    type Output = Circuit;

    fn name(&self) -> &'static str {
        "pirtm-lower"
    }

    fn run(&self, input: PirtnProgram) -> Result<Circuit, CircuitError> {
        input.lower_to_circuit().map_err(|_| CircuitError::EmptyCircuit)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_pirtm_lower_matmul() {
        let mut prog = PirtnProgram::new(4);
        prog.add_op(TensorOp::MatMul { a: 0, b: 1, c: 2 });
        let circuit = prog.lower_to_circuit().unwrap();
        assert!(circuit.validate().is_ok());
        assert_eq!(prog.op_count(), 1);
        assert_eq!(prog.qubit_count(), 4);
    }

    #[test]
    fn test_pirtm_lower_add() {
        let mut prog = PirtnProgram::new(3);
        prog.add_op(TensorOp::Add { a: 0, b: 1, out: 2 });
        let circuit = prog.lower_to_circuit().unwrap();
        assert!(circuit.validate().is_ok());
    }

    #[test]
    fn test_pirtm_lower_contract() {
        let mut prog = PirtnProgram::new(3);
        prog.add_op(TensorOp::Contract { a: 0, b: 1, out: 2, axis: 0 });
        let circuit = prog.lower_to_circuit().unwrap();
        assert!(circuit.validate().is_ok());
    }

    #[test]
    fn test_pirtm_lower_permute() {
        let mut prog = PirtnProgram::new(3);
        prog.add_op(TensorOp::Permute { input: 0, out: 1, axes: vec![1, 0, 2] });
        let circuit = prog.lower_to_circuit().unwrap();
        assert!(circuit.validate().is_ok());
    }

    #[test]
    fn test_pirtm_lower_scalar_mul() {
        let mut prog = PirtnProgram::new(2);
        prog.add_op(TensorOp::ScalarMul { tensor: 0, out: 1, scalar: 2 });
        let circuit = prog.lower_to_circuit().unwrap();
        assert!(circuit.validate().is_ok());
    }

    #[test]
    fn test_pirtm_lower_multi_op() {
        let mut prog = PirtnProgram::new(4);
        prog.add_op(TensorOp::MatMul { a: 0, b: 1, c: 2 });
        prog.add_op(TensorOp::Add { a: 2, b: 3, out: 0 });
        let circuit = prog.lower_to_circuit().unwrap();
        assert!(circuit.validate().is_ok());
        assert_eq!(prog.op_count(), 2);
    }

    #[test]
    fn test_pirtm_empty_program() {
        let prog = PirtnProgram::new(2);
        let result = prog.lower_to_circuit();
        assert!(result.is_err());
    }

    #[test]
    fn test_pirtm_out_of_bounds() {
        let mut prog = PirtnProgram::new(2);
        prog.add_op(TensorOp::MatMul { a: 0, b: 1, c: 5 });
        let result = prog.lower_to_circuit();
        assert!(result.is_err());
    }
}

