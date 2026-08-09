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

Use `./scripts/ace-build build` for builds. It serializes concurrent CMake
builds for the same build directory and reports when it is waiting for another
build to finish. Behavior tests use the same lock automatically.

## Symbol Declaration and Lifecycle

### Source declarations

Parsing creates syntax trees and their scope tree before symbols are declared.
`Application::CollectSyntaxes(...)` flattens each owned AST into borrowed
`ISyntax*` pointers. The ASTs and source buffers remain alive for the whole
compilation, so syntax and `SrcLocation::Buffer` pointers remain valid while
symbols are created, diagnosed, bound, and emitted.

`Application::CreateAndDeclareSymbols(...)` filters the flattened syntax list
to `IDeclSyntax` nodes and sorts them by `DeclOrder` and `GetDeclSuborder()`.
This ordering lets declarations such as modules and types exist before later
declarations resolve them.

Source declarations enter through:

```cpp
Scope::DeclareSymbol(const IDecl* decl)
```

That overload has two paths:

- An `IPartialDecl`, currently a module, may find an existing symbol and call
  `ContinueCreatingSymbol(...)`. This returns before `CreateSymbol()`.
- Every other declaration calls `decl->CreateSymbol()`, collects its
  diagnostics, and forwards the resulting `unique_ptr<ISymbol>` to the owning
  declaration overload.

Source-only declaration rules belong in this overload. They must cover the
partial-declaration early return as well as the normal `CreateSymbol()` path.
Source provenance is available through an `IDeclSyntax` source location; do
not infer it from a package name or path.

### Owned declaration

All symbol ownership enters a scope through:

```cpp
Scope::DeclareSymbol(std::unique_ptr<TSymbol> symbol)
```

The symbol determines its destination with `GetScope()`. For body-scoped
symbols, `GetScope()` is the parent of `GetBodyScope()`. The overload checks
for a declaration with the same name, generic arguments, and prototype self
type. After reporting a redeclaration, it returns the existing symbol and
destroys the new candidate when the existing symbol can be cast to `TSymbol`.
The source path forwards `unique_ptr<ISymbol>`, so any matching existing symbol
is returned. Calls using a concrete generated `TSymbol` may instead insert an
incompatible conflicting symbol for recovery. Callers must use the returned
pointer and must not assume the pointer passed into the function survives.

On successful insertion, the destination scope owns the symbol in:

```cpp
std::unordered_map<std::string, std::vector<std::unique_ptr<ISymbol>>>
```

`OnSymbolDeclared(...)` then registers applicable generic roots with that
scope's `GenericInstantiator`. Raw `ISymbol*` values returned by declaration
and resolution are borrowed pointers into this scope-owned storage.

The owning overload is also used directly for compiler-created symbols:

- built-in, native, void, and error symbols;
- generic instances and instantiated body symbols;
- generated copy/drop glue functions and their parameters;
- lowering-created temporary and label symbols.

Do not place source-language-only validation in the owning overload unless it
must also apply to all of these generated symbols.

### Binding and generated symbols

After initial declaration, `BindSymbolParents(globalScope)` binds borrowed
parent pointers from type parameters to their owner and from parameters to
their callable. These links are not ownership links.

`FunctionBlockBinding` temporarily pairs a declared `FunctionSymbol*` with an
owned optional block syntax. A binding is kept only when the returned symbol's
body scope is the syntax's body scope, preventing a redeclaration from binding
its body to an earlier function. Function bodies are converted to checked and
lowered semas later and stored by `FunctionSymbol::BindBlockSema(...)`.

Generic instantiation calls `CreateInstantiated(...)`, declares the resulting
owned symbol through the `unique_ptr` overload, recursively instantiates body
symbols into new child scopes, and binds their parents. Referenced
monomorphizations can therefore add symbols after initial source declaration.

Glue generation follows the same generated path: it creates function and
parameter symbols, declares them through the owning overload, binds parents,
then attaches an emit-ready block. Emission only borrows the resulting symbols.

### Lifetime and removal

A scope owns its symbols, while body-scoped symbols and syntax nodes keep child
scopes alive with `shared_ptr`. Child scopes hold their parent with
`shared_ptr`; parents track children with `weak_ptr`. `GlobalScope::~GlobalScope`
calls `Scope::Clear()` recursively to clear symbol maps and parent links and
break the resulting ownership graph.

Symbols commonly store raw pointers to other symbols and semas commonly store
borrowed symbol pointers. These are valid only while the owning scope tree is
alive. `Scope::RemoveSymbol(...)` immediately destroys the selected symbol and
invalidates every borrowed pointer to it; use it only when no references can
remain. It currently has no production call sites.

## Formatting

C++ sources are formatted with the `clang-format` binary from the configured
LLVM toolchain. Run `./scripts/ace-build build --target format` to apply
formatting and `./scripts/ace-build build --target check-format` to verify it.

## C++ Code Rules

After a `dynamic_cast` that is expected to succeed by an established compiler
invariant, immediately `ACE_ASSERT` the result. Do not assert casts used for
type probing, optional branching, diagnostics, or recovery.

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
