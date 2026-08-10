# Evolution And Design Evidence

Ace's history is useful because refactors often state intent more clearly than unfinished current
code. Use history to understand why a boundary exists, not to restore every previous behavior.

## Major Architectural Refactors

### Shared immutable nodes (`731892b`)

Node construction moved from unique immutable objects to `shared_ptr<const ...>`, enabling passes
to return an existing node when unchanged while safely sharing immutable subtrees. Later sema APIs
retain this pattern.

### Dedicated analysis and infallible lowering (`395dea`, `ffe5bf5`, `f822af1`)

Control-flow analysis moved out of block nodes into a dedicated layer; statements began producing
linear control-flow data consumed by that layer. Lowering was separately made non-diagnostic. These
changes are historical evidence that validation, transformation, and analysis are distinct phases.

### Syntax, sema, traits, and generics (`bb07576`, 2023)

This large cutover renamed generic AST "nodes" to syntaxes, bound nodes to semas, templates to
generics, and introduced the current trait/generic/scope structure. The breadth of the change is
evidence that representation names and phase separation were deliberate, not incidental style.

### Reference legality moved to type checking (`3b94abd`)

`CastExprSema` was introduced so syntax binding could preserve a cast/reference operation while
`CreateTypeChecked()` enforced conversion and reference-binding rules. This is the strongest
historical example for deciding where a new semantic check belongs.

### Parent symbol relationships (`d62cc20`, `2807829`, `ca9cf65`)

Parameters and type parameters gained explicit borrowed parent references. Follow-up fixes moved
binding to safe orchestration points after source declaration, generic body instantiation, and glue
declaration. The lesson is not merely "have parent pointers": relationships must be bound only when
owners are stable and must not create ownership cycles.

### Scope lifetime cleanup (`3883c8d`, `afb5d1e`)

Scope ownership evolved toward strong parent links and weak child tracking, followed by explicit
global-scope clearing to fix memory leaks. Current teardown is part of the ownership design, not
incidental destructor code.

### Control-flow instruction naming (`a1e7ede`, `7b9cf95`)

The initial rename changed the type but missed APIs and locals; the follow-up completed the cutover.
This establishes a review rule: a conceptual rename must update the vocabulary at every layer, not
only the central type.

### Trait-member and operator recovery (`45ef9dc`, `4084476`)

Trait member lookup moved from synthetic trait scopes toward concrete matching impl scopes.
Operator lowering and conversions were then repaired to support trait-resolved functions. These
commits show the trait-first direction, but current unary lookup remains native and binary lookup
still combines native and trait candidates. The one-path operator cutover is not complete.

### Embedded standard library (`fc6cd7f`)

Std declarations moved from manually embedded C++ strings to `.ace` files embedded at build time.
This preserved a self-contained compiler while recovering readable source, source locations, and
validation. Later commits prefixed compiler-owned names (`51cb051`) and reserved that prefix based
on source origin (`77ebfc3`).

### Diagnostic and test hardening

Recent changes added behavior tests, runtime smoke coverage, native verification, display-name
architecture checks, and safer diagnosed fallbacks. These changes show that compiler success is an
end-to-end property: build, diagnostics, emitted behavior, and naming conventions all matter.

Earlier diagnostic history supports the same model: `982b338` spread diagnostics across compiler
passes, `9621284` reduced cascades after error-typed expressions, and `4e4ecea` made deliberately
discarded diagnostics use `CreateNoError()` rather than unchecked unwrapping. `729f259` later turned
missing native declarations into an early diagnostic instead of optional-access failure.

### Semantic LLVM naming (`d3c0073`, `70e37d1`)

LLVM-facing locals were renamed from vague terms such as `var`, `value`, and `i` to roles such as
`globalVar`, `ptr`, and `indexPtr`. This extends the naming principle through backend code: names
should describe semantic roles even when the type is uniformly `llvm::Value*`.

## Reading Historical Evidence

When investigating a feature:

1. Locate the current implementation and tests.
2. Use `git log -- <paths>` to find structural changes.
3. Read both the introducing commit and later repair commits.
4. Treat a completed strict rename/refactor as strong evidence.
5. Treat checkpoint commits, TODO comments, and code repaired immediately afterward as weak
   evidence.
6. Prefer the latest coherent model over compatibility with abandoned intermediates.

## Known Tensions

These areas need explicit design work rather than pattern matching:

- `Lifetime` exists in std and glue exists in the compiler, but the final default/copy/drop model is
  still on the TODO list.
- Operators are trait-oriented, while logical negation and short-circuit logic retain dedicated
  semas. Unary operators still use native maps, and binary lookup combines native and trait
  candidates.
- Binary operator argument convertibility is currently checked during sema creation despite the
  broader rule that conversion legality belongs in type checking.
- Trait member lookup has a clear impl-first direction, but old `CollectImplOfFor` helpers and TODO
  comments remain.
- Standard-library behavior is increasingly expressed in Ace, while conversions and many native
  maps still live in C++.
- Compiler output uses LLVM libraries for IR but external `llc` and `clang` for final artifacts.
- The code strives for recoverable diagnostics, but old optional/assertion paths may still encode
  assumptions that invalid source can violate.
- Failure behavior tests match required diagnostic substrings but do not enforce diagnostic count
  or ordering, so duplicate-diagnostic regressions need location/count-aware coverage.

Document a resolution when one of these tensions is intentionally settled. Until then, label it as
an open edge and ask for the desired language semantics before making a broad refactor.

## Documentation Maintenance

Update these documents when a change alters a phase contract, ownership relationship, resolution
rule, public language semantic, native boundary, or declared project direction. Small mechanical
changes do not need historical entries. Architectural changes should record the commit and the
reasoning it made durable.
