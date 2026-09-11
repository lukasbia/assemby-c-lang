# Assenbly-C

A Swift-inspired systems language with explicit long-form operations, ARC-oriented semantics,
a dedicated ACIL middle IR, and an LLVM-oriented backend. The compiler implementation is C++20.

## Current bootstrap implementation

This repository is a complete, flat-file bootstrap compiler rather than a production implementation
of every planned Swift feature. It includes:

- lexer with exactly 95 reserved keywords
- recursive-descent parser
- AST
- semantic/type checking
- ACIL IR and verifier
- ACIL optimization pass
- textual LLVM IR emission for a useful integer subset
- compiler driver
- package manifest parser/validator
- formatter-friendly source conventions
- tests and example program

## Build

```sh
cmake -S . -B build
cmake --build build
```

Run:

```sh
./build/assenblyc examples/hello.ac --emit-acil
./build/assenblyc examples/hello.ac --emit-llvm
./build/assenblyc examples/hello.ac --check
```

The initial backend emits textual LLVM IR. Linking to a native executable can be added by invoking
the platform LLVM toolchain after LLVM IR generation.

## Language example

```assenbly-c
module Hello

func main() -> Int32 {
    let x: Int32 = 10
    let y: Int32 = 20
    let total: Int32 = x + y
    return total
}
```

Assenbly-C deliberately does not have compound assignment operators such as `+=`, `-=`, `*=`, or `/=`.
Write the full expression instead:

```assenbly-c
x = x + 1
```

## Project organization

The repository is intentionally divided into many folders:

- `compiler/frontend/lexer` — lexer
- `compiler/frontend/parser` — parser
- `compiler/frontend/ast` — AST
- `compiler/frontend/sema` — semantic analysis
- `compiler/middleend/acil` — ACIL generation and verification
- `compiler/middleend/optimizer` — ACIL optimization
- `compiler/backend/llvm` — LLVM IR lowering
- `compiler/backend/codegen` — native code generation
- `compiler/backend/linker` — linker integration
- `compiler/driver` — compiler driver
- `runtime` — ARC and runtime implementation
- `stdlib` — standard library
- `package` — package system
- `tools` — ACPM, formatter, and documentation tools
- `tests` — compiler and integration tests
- `examples` — example programs
- `docs` — language and toolchain documentation
- `scripts` — development scripts
