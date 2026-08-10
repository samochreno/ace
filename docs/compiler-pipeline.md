# Compiler Pipeline

`src/Application.cpp` is the authoritative top-level orchestration. The pipeline is intentionally
visible and mostly linear.

## Current Compilation Flow

1. `Compilation::Parse(...)` interprets CLI arguments and package metadata and reads user files.
2. Embedded standard-library `FileBuffer`s and user `FileBuffer`s are parsed into `ModSyntax` ASTs.
3. `Application::CollectSyntaxes(...)` flattens each owned syntax tree into borrowed pointers.
4. `CreateAndDeclareSymbols(...)` selects declaration syntaxes, orders them, and declares symbols.
5. `BindSymbolParents(...)` connects type parameters and parameters to their declared owners.
6. Public-interface leaks are diagnosed after declarations exist.
7. Generic bodies are instantiated and root generic bodies are prepared.
8. Native declarations are verified. Declaration/native errors stop before function-body binding.
9. Function blocks run `CreateSema()`, `CreateTypeChecked()`, and `CreateLowered()`, then bind to
   their `FunctionSymbol`.
10. Non-void functions produce control-flow instructions and are validated through a
    `ControlFlowGraph`.
11. Referenced monomorphizations are instantiated to a fixed point.
12. Copy/drop glue symbols and emit-ready blocks are generated, followed by another mono pass.
13. Whole-compilation diagnoses validate layout cycles, orphan impls, trait impls, overlapping
    inherent impls, concrete constraints, and supertraits.
14. Any remaining error stops emission.
15. `Emitter` creates LLVM IR, optimizes it, writes artifacts, calls `llc`, and links with `clang`.

The two explicit error gates matter. Early declaration errors can invalidate body assumptions;
late semantic diagnoses can invalidate emission assumptions. Do not move emission earlier merely
because an individual node can generate LLVM.

## Phase Ownership

| Phase | Owns | Must not own |
| --- | --- | --- |
| Lexing/parsing | tokens, grammar, syntax shape, source spans, scope-tree construction | symbol resolution, type legality |
| Declaration | creating and owning symbols, declaration order, duplicate/access/signature rules | function-body expression typing |
| Sema creation | binding names, types, callables, fields, operators, impl members | value-category and conversion legality |
| Type checking | `TypeInfo`, `ValueKind`, conversions, assignment/call/return compatibility | syntax parsing, LLVM emission |
| Lowering | replacing valid high-level semas with simpler semas | new source-language validity rules |
| Control flow | path structure and non-void termination | name/type resolution |
| Mono/glue | materializing referenced generic and lifetime behavior | source parsing |
| Whole-program diagnoses | relationships requiring the declared program as a whole | backend representation |
| Emission | LLVM representation, temporaries, drops, output artifacts | new language semantics |

## Important Stage Invariants

- Syntax trees and source buffers outlive every borrowed syntax/source-location pointer.
- Declaration ordering ensures modules and types exist before dependent declarations are created.
- A bound function body is already type checked and lowered.
- Emission sees no user-facing semantic errors and may assert invariants established earlier.
- `CollectMonos()` observes semantic references without mutating the tree.
- Generic and glue generation can add symbols after initial declaration, so ownership rules must
  work for both source and generated symbols.

## Adding A Rule

Use the information test:

- Is the text structurally invalid? Parsing.
- Can the declaration or referenced symbol not be formed? Declaration or sema creation.
- Are two resolved types/values illegal together? Type checking.
- Is a valid operation still too high-level for emission? Lowering.
- Is a path missing or non-terminating? Control-flow validation.
- Does validity depend on the complete set of impls/types? Whole-program diagnosis.
- Is the valid operation being translated to LLVM? Emission.

The reference-binding refactor in `3b94abd` is the clearest precedent: the parser/sema-creation path
kept a `CastExprSema`, while lvalue and reference-binding legality moved to `CreateTypeChecked()`.

## Current Deviations

The phase table describes the intended contract, but current code is not perfectly aligned:

- `ResolveBinaryOpSymbol()` checks argument convertibility while operator syntax is creating its
  sema. That legality belongs in type checking under the broader pass model.
- Some high-level semas expose methods that are deliberately unreachable because lowering must
  remove them before those phases.
- Native operator maps remain a semantic resolution path even though trait-first operators are the
  intended direction.

Treat these as migration points. Do not generalize them into permission for new binding-time type
checks or parallel resolution systems.
