# Design Principles

## Prefer A Linear Compiler

Ace is organized as a sequence of increasingly strong representations. Each stage should do one
kind of work, establish an invariant, and hand a simpler problem to the next stage. Later phases
must not repeat parsing or name resolution, and emission must not invent source-language meaning.

This is why `Application.cpp` visibly orchestrates declaration, binding, type checking, lowering,
control-flow validation, monomorphization, glue, whole-program diagnoses, and emission instead of
hiding those transitions inside the backend.

When adding a rule, put it in the earliest phase that has all required information, not the earliest
place where it is convenient to report an error.

## Preserve Meaning With Explicit Representations

Source structure is represented by syntax objects. Resolved meaning is represented by semas.
Declarations become symbols owned by scopes. Linear control-flow facts become
`ControlFlowInstruction`s. LLVM values exist only during emission.

Do not use one representation as a vague container for several stages. The 2023 refactor in
`bb07576` deliberately replaced generic "nodes" and "bound nodes" with the more precise syntax and
sema vocabulary. The later `ControlFlowNode` to `ControlFlowInstruction` refactor (`a1e7ede`,
`7b9cf95`) reinforced the same rule: names should say what a thing is in the pipeline.

## Transform Immutably

Syntax and sema trees are generally held as `shared_ptr<const T>`. Type checking and lowering
transform children first, return the existing node when unchanged, and construct a replacement
when anything changes. This makes pass boundaries visible and prevents one stage from silently
invalidating another stage's view.

Immutability does not mean the whole compilation is purely functional. Scopes accumulate symbols,
generic instantiation adds instances, function symbols bind their final bodies, and emitters hold
backend state. Mutation is concentrated in objects whose role is ownership, orchestration, or
output, rather than spread through semantic trees.

## Make Ownership Obvious

Scopes own symbols with `unique_ptr`. Syntaxes and semas own child nodes with
`shared_ptr<const ...>`. Raw symbol pointers are borrowed references into the scope tree. Child
scopes keep parents alive; parents track children weakly and are explicitly cleared at global
teardown.

New raw pointers require an ownership explanation. New shared ownership should correspond to a
real lifetime relationship, not uncertainty about who owns an object. Compiler-generated symbols
must enter the same scope ownership path as source-generated symbols.

The immutable syntax/sema child graphs are acyclic, but the wider live compilation graph is not
strictly acyclic: a scope can own a function symbol that retains a sema which retains that scope,
and body-scoped symbols retain child scopes that retain their parents. These cycles are localized
and broken explicitly by global-scope teardown. Do not introduce another cycle without identifying
both why it is required and where it is broken.

## Use The Language To Build The Language

Ace is intended to become self-hosting: the end state is an Ace compiler implemented in Ace. The
current C++ compiler is a bootstrap implementation, so its architecture should be straightforward
to express in Ace rather than built around incidental C++ complexity.

Functionally, Ace deliberately covers a simpler subset of the model used by the C++ implementation.
Ace values, functions, structs, traits, references, ownership, optionals, containers, and control
flow should have unsurprising C++ representations. An Ace program should be straightforward to
translate to C++; translating arbitrary C++ to Ace is neither expected nor desired.

Ace prefers language-visible definitions with compiler-provided primitives beneath them. Operator
traits live in `std/op.ace`; integer and float impls call native externs; reference-counted pointer
behavior lives in `std/rc.ace`. Standard-library source is embedded into the executable at build
time rather than rewritten as C++ declarations.

Compiler internals still obey declaration, scope, type, generic, and function rules. The `__`
prefix marks compiler-owned source declarations; source origin, not a spoofable package name or
path, controls that privilege.

Avoid a second semantic path for built-ins when ordinary traits, impls, calls, conversions, or
symbols can express the behavior. Native code should supply irreducible operations, not a parallel
language.

Self-hosting is a direction, not permission to fake capabilities the language does not yet have.
Bootstrap-only C++ and LLVM boundaries may remain explicit until Ace can represent them honestly.
For ordinary compiler logic, prefer data flow, ownership, and abstractions that have a natural Ace
equivalent.

## Treat Diagnostics As Data

Operations return values together with diagnostics. Recoverable operations use error symbols or
placeholder semas so independent errors can still be found. Fatal operations use an absent
`Expected` value to stop a path cleanly. Unchecked optional access and backend crashes are not error
handling.

Diagnostics belong to the phase that owns the rule. Assertions document compiler invariants; they
must not replace diagnostics for invalid user programs.

## Prefer One Source Of Truth

Resolution, conversion, and operator behavior should have one canonical path. Strict refactors are
preferred over aliases and compatibility fallbacks. When a feature changes direction, remove the
old model rather than layering another model over it.

The trait-member refactor (`45ef9dc`) is a useful precedent: instance trait members resolve through
applicable impl bodies. The operator restoration (`4084476`) shows the desired trait-backed
direction, but current operator lookup still mixes native maps and trait impls. It is an open
cutover, not an established one-path contract.

## Consistency Is Semantic

Names such as `Syntax`, `Sema`, `Symbol`, `Instruction`, `Self`, and `self` communicate invariants.
Diagnostic result wrappers communicate whether recovery is possible. Source location ownership
communicates whether a declaration is user or compiler supplied. In Ace, inconsistent naming and
error patterns are not merely cosmetic because they obscure which model applies.

## Keep Direction Separate From Completion

Ace contains incomplete work: lifetime traits, strings, associated types, logical negation traits,
dynamic casting, data-flow analysis, DLL metadata, and external toolchain handling. Existing
fragments are evidence of direction, not necessarily final semantics. Extend them only after
confirming the intended language rule and its place in the pipeline.
