# Nodes And Passes

## Representation Vocabulary

### Syntax

`ISyntax` represents parsed source. Every syntax provides a source location, a scope, and recursive
child collection. Syntaxes are immutable and commonly stored as `shared_ptr<const T>`.

`IDeclSyntax` also implements `IDecl` and can create a symbol. `ISemaSyntax<T>` can bind itself into
a semantic node. Some compiler-created syntax objects exist inside parsed trees, such as
`TypeReimportSyntax`; being a syntax does not by itself mean the user wrote that declaration.

### Symbols

Symbols represent declared identities: modules, types, traits, impls, functions, prototypes,
parameters, fields, variables, constraints, and imports. They are not AST nodes. Symbols are owned
by scopes and are the stable identities referenced by semas.

### Semas

`ISema` represents resolved program meaning. Semas retain source locations and scopes, but refer to
resolved symbols and expose monomorphization dependencies. Expression semas additionally expose
`TypeInfo`; statement semas can produce control-flow instructions and emit behavior.

### Control-flow instructions

`ControlFlowInstruction` is a linear validation record, not a tree node. Statement semas produce
labels, jumps, conditional jumps, returns, and exits. A separate `ControlFlowGraph` validates path
behavior.

### Emittables

`IEmittable<T>` is the final LLVM-facing contract. Most checked/lowered semas are emittable, but
compiler-generated glue may bind a purpose-built emittable block without pretending it came from
source syntax.

## Immutable Transformation Pattern

For `CreateTypeChecked()` and `CreateLowered()`:

1. Transform children first.
2. Collect diagnostics if the pass can diagnose.
3. If all children and semantic choices are unchanged, return `shared_from_this()`.
4. Otherwise construct the same semantic node with transformed children.
5. Preserve source location, scope, and resolved symbols.
6. If rebuilding exposes another lowering opportunity, recurse on the replacement deliberately.

This pattern gives identity reuse without mutation and makes changed subtrees explicit. A pass
must not cast away constness to update an existing syntax or sema.

## Pass Contracts

### `CreateSema()`

- Recursively binds child syntax.
- Resolves names, types, variables, fields, callables, operators, and impl members.
- Produces a semantic operation even when a later type decision remains.
- Uses error symbols/placeholders when binding can recover.

It should not enforce lvalue/rvalue relationships or insert conversions based on `TypeInfo`.

### `CreateTypeChecked()`

- Recursively checks children.
- Validates `TypeInfo` and `ValueKind` relationships.
- Inserts implicit or explicit conversion semas.
- Validates assignment, call arguments, return values, and reference binding.
- Rebuilds immutable nodes when checking changes children.

Statements receive `StmtTypeCheckingContext`, including the parent function return type;
expressions use `TypeCheckingContext`.

### `CreateLowered()`

- Assumes semantic validity.
- Rewrites source-level convenience into simpler operations.
- Does not collect diagnostics.
- Leaves emission with explicit, established semantics.

Compound assignment is a useful example: it must preserve one resolved operator path and one
assignment target while lowering into simpler operations, rather than resolving the operator again
inside emission.

Several high-level semas deliberately make `Emit()`, mono collection, or control-flow creation
unreachable. This is an executable assertion that lowering must remove them. `GroupStmtSema` can
also contain expandable statements whose final statement sequence is expanded at emission after
semantic lowering; this is controlled generation, not permission for general semantic decisions in
the emitter.

### `CollectMonos()`

`MonoCollector` follows semantically relevant child semas and generic/typed symbols. It records
placeholder generic dependencies without changing the tree. Every new sema must collect from all
children or symbols that can cause a referenced monomorphization.

### `CreateControlFlowInstructions()`

This runs on lowered statement semas. Blocks concatenate child instruction sequences; structured
statements contribute the labels and jumps needed to represent their paths. Validation happens
outside the nodes in `InvalidControlFlowDiagnosis`.

### `Emit()`

Emission consumes the checked/lowered representation. It may manage LLVM blocks, allocas,
temporaries, copy/drop metadata, and vtables. It must not perform name lookup or decide whether a
source conversion is legal.

## Tree And Lifetime Shape

- Syntax and sema parents own immutable children.
- Semas borrow symbols owned by scopes.
- Syntax/sema source locations borrow source buffers held for the compilation lifetime.
- Function symbols bind their final sema/emittable bodies after declaration rather than owning
  mutable syntax nodes.
- Generated labels and temporaries are symbols even though no source declaration created them.

The node child graphs are acyclic. The surrounding compilation ownership graph can contain
localized cycles through scopes, symbols, and bound semas; global scope clearing is the explicit
teardown boundary for those relationships.

Cycles should not be introduced casually. Parent links on parameter/type-parameter symbols are
borrowed raw pointers, not shared ownership. The parent-binding refactors (`d62cc20`, `ca9cf65`)
made those relationships explicit and bound them only after declaration/instantiation.

## New Node Checklist

- Does its name identify its representation and role?
- Does `CollectChildren()` include every owned syntax child?
- Does `CreateSema()` preserve unresolved operations needed by type checking?
- Does type checking transform every semantic child and preserve metadata?
- Does lowering return the same object when unchanged?
- Does `CollectMonos()` mirror every semantically relevant dependency?
- Does statement control-flow output match lowered behavior?
- Does emission assume, rather than recreate, earlier semantic decisions?
