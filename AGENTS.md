# Ace Compiler Guide

## Compiler Pipeline

Ace compiles source in these broad stages:

1. Lex and parse source files into `ISyntax` trees.
2. Collect declaration syntaxes and declare their symbols with
   `Scope::DeclareSymbol(...)` / `CreateSymbol()`.
3. Instantiate required generic declarations and bodies.
4. Convert function-body syntax into semantic nodes with `CreateSema()`.
5. Validate and convert semantic nodes with `CreateTypeChecked()`.
6. Rewrite checked semantic nodes into emit-ready form with `CreateLowered()`.
7. Build and validate control-flow graphs for non-void functions.
8. Instantiate referenced monomorphizations and generate glue.
9. Emit LLVM IR only when no errors remain.

The orchestration lives in `src/Application.cpp`. Keep stage boundaries intact;
later passes may rely on invariants established by every earlier pass.

Behavior tests invoke Ninja themselves, so run the full CTest suite serially
(`ctest --test-dir build --output-on-failure -j1`) to avoid concurrent Ninja
dependency-log failures.

## Pass Responsibilities

### Parsing

The parser creates syntax nodes and reports token or grammar errors. Syntax
nodes preserve source locations, scopes, and child structure. Parsing should
not resolve symbols or make semantic type decisions.

### Declaration

`CreateSymbol()` and `Scope::DeclareSymbol(...)` create symbols and report
declaration errors such as invalid names, duplicate declarations, inaccessible
types, or malformed declaration signatures. Function bodies are bound later,
after declarations are available.

### `CreateSema()`

`CreateSema()` binds syntax to semantic meaning. It should:

- recursively create child semas;
- resolve names, types, fields, callables, operators, and impl members;
- select the symbol represented by the syntax;
- report binding or resolution diagnostics;
- return an immutable sema tree, using error symbols when recovery is needed.

Do not put value-category checks, implicit-conversion legality, or other
type-checking rules in `CreateSema()`. If a rule depends on `TypeInfo`,
`ValueKind`, or the relationship between source and target types, it normally
belongs in `CreateTypeChecked()`.

Syntax that needs a later type-checking decision should preserve that operation
as a sema node. For example, `CastExprSyntax` creates `CastExprSema`, which
performs explicit conversion and reference-binding checks during type checking.

### `CreateTypeChecked()`

`CreateTypeChecked()` establishes type correctness. It should:

- recursively type-check children;
- insert implicit or explicit conversion semas;
- enforce assignability and `ValueKind` rules;
- validate argument, return, operator, and assignment types;
- report diagnostics from `Diagnostics/TypeCheckingDiagnostics`;
- return `shared_from_this()` when unchanged, otherwise rebuild the immutable
  node with checked children.

Type checking may return error/placeholder semas to keep diagnostic collection
running. It must not lower syntax sugar or emit LLVM IR.

Statement nodes receive `StmtTypeCheckingContext`, including the parent
function return type. Expression nodes use `TypeCheckingContext`.

### `CreateLowered()`

`CreateLowered()` assumes the tree has already been type-checked. It should:

- recursively lower children;
- replace high-level constructs with simpler sema nodes;
- preserve source locations, scopes, and established types;
- rebuild only when something changed;
- recurse on rebuilt nodes when another lowering step may be required.

Lowering returns a sema directly and does not collect diagnostics. Do not add
new source-language validity checks here.

### `CollectMonos()` and Glue

`CollectMonos()` walks a sema and collects referenced placeholder generic or
typed symbols. It should mirror the node's semantically relevant children and
symbols without changing the tree. Generic instantiation and glue generation
are coordinated globally after function bodies are bound.

### Control Flow

`CreateControlFlowNodes()` runs on lowered statement semas. It produces a
linear sequence describing labels, jumps, conditional jumps, returns, and
exits. `BlockStmtSema` concatenates child sequences; individual control-flow
statements contribute their own instruction. `ControlFlowGraph` validation is
performed outside the nodes after lowering.

The current `ControlFlowNode` name means a linear control-flow instruction, not
an AST node. Keep APIs and local naming consistent if this type is renamed.

### Emission

`Emit()` consumes checked, lowered, and validated semas. Node emission should
translate established semantics to LLVM IR and manage temporary/drop metadata;
it should not resolve symbols or make new language-level type decisions.
Backend or output failures may still be diagnosed by the emitter itself.

## Node Implementation Pattern

For transformation passes, follow the existing immutable pattern:

1. Create a local `DiagnosticBag` when the pass can diagnose.
2. Transform children and collect their diagnostics.
3. Return `shared_from_this()` when every child is unchanged.
4. Otherwise construct the same node type with transformed children.
5. Preserve source location, scope, and resolved symbols.

Use `DiagnoseX(...) -> Diagnosed<void>` for reusable validation and
`CreateXError(...) -> DiagnosticGroup` for constructing a concrete diagnostic.
Keep diagnostics in the file matching the pass (`Parsing`, `Binding`,
`TypeChecking`, and so on).

When deciding where a new rule belongs, use this test:

- Is the source text structurally valid? Parsing.
- Can names and symbols be resolved? Declaration or `CreateSema()`.
- Are the resolved values and types legal together? `CreateTypeChecked()`.
- How should a valid construct be represented more simply? `CreateLowered()`.
- Is every path valid and terminating? Control-flow validation.
- How is the validated operation represented in LLVM? `Emit()`.
