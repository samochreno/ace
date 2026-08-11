# C++ Style And Idioms

Ace uses C++20 as an implementation language, but its C++ style is shaped primarily by the
compiler's architecture. Types and APIs should make phase, ownership, mutability, optionality, and
failure behavior visible. Prefer a direct implementation over a clever abstraction when both
express the same contract.

This document records established patterns in the current code and durable refactor history. It is
not a mandate to make unrelated code uniform.

## Write C++ That Can Become Ace

The end goal is to implement the Ace compiler in Ace. The current C++ implementation should make
that rewrite straightforward rather than depend unnecessarily on machinery that has no natural Ace
equivalent.

Ace is functionally a deliberately simplified subset of the C++ model used by this compiler. Its
syntax and mechanics are different, but familiar values, functions, structs, traits, references,
shared ownership, optionals, containers, and explicit control flow should map cleanly into C++.
Ace-to-C++ translation should therefore be unsurprising; translating arbitrary C++ into Ace is not
a goal.

This is a design preference, not a demand to pretend Ace is already self-hosting. The bootstrapping
compiler may still need C++ and LLVM facilities that Ace cannot yet express. When two C++ designs
are otherwise equivalent, prefer the one whose ownership, data flow, and abstraction could be
represented directly in Ace.

## Names Describe Compiler Roles

Types and functions use `PascalCase`; locals and parameters use `camelCase`; data members use an
`m_` prefix. Interfaces commonly use an `I` prefix, as in `ISyntax`, `ISema`, and `ISymbol`.
Names should identify the representation and its place in the pipeline rather than use generic
terms such as "node" when a more precise term exists.

Function prefixes carry recurring meanings:

- `Get...` returns existing state and should not imply a new semantic object;
- `Create...` constructs a value, node, diagnostic, or transformed representation;
- `Collect...` traverses or accumulates all relevant items;
- `Resolve...` performs semantic lookup and can return diagnostics;
- `Diagnose...` performs a reusable validation and returns diagnostics;
- `Is...` and `Has...` answer predicates;
- `Emit...` performs backend generation after semantic validity is established.

Getters that expose owned objects commonly return a reference to the owning smart pointer or
container, while borrowing APIs return raw pointers. Shared-pointer getters are not fully
consistent about returning by value or reference, so preserve the local lifetime contract instead
of performing a broad mechanical rewrite.

Functions use trailing return types:

```cpp
auto Scope::FindMod() const -> std::optional<ModSymbol*>;
```

Use the vocabulary already established for the relevant compiler layer: `Syntax`, `Sema`,
`Symbol`, `Scope`, `Instruction`, `TypeInfo`, and `SrcLocation` communicate architectural
contracts. A strict rename should update types, functions, locals, and documentation together.

## Const And Immutability

Constness is part of Ace's representation model, not decoration.

- Syntax and sema children are normally `std::shared_ptr<const T>`.
- Transformation methods are `const` and create a replacement representation rather than mutate
  an existing tree.
- APIs take `const T&` when borrowing a non-null value for the duration of a call.
- Raw pointer variables commonly use `T* const` when the pointer itself is not reassigned.
- Use `const auto`, `auto* const`, and `const auto&` when they preserve the relevant value,
  pointer, or reference constness without repeating a long type.

Mutation belongs in objects whose explicit job requires it: scopes own declarations, generic
instantiators materialize instances, diagnostic bags accumulate errors, and emitters accumulate
backend state. Do not make semantic trees mutable to avoid rebuilding a changed node.

Immutability is a tree and pass-boundary rule, not a requirement to heap-allocate every compiler
value. Tokens, names, contexts, type information, and other small values are routinely stored and
passed by value. Lazy caches may use controlled interior mutation when the observable semantic
value remains unchanged.

`const_cast` exists in a few localized symbol-instantiation paths. It is not a general escape hatch.
New uses need a specific lifetime or API-boundary justification and should prompt consideration of
whether the surrounding interface has the wrong constness.

## Ownership And Lifetimes

Pointer choice communicates ownership:

- `std::unique_ptr<T>` represents exclusive ownership. Scopes own symbols this way.
- `std::shared_ptr<const T>` represents shared ownership of immutable syntax and sema trees.
- `std::shared_ptr<Scope>` represents the explicit shared lifetime of scopes.
- `std::weak_ptr<T>` breaks known ownership cycles, notably parent tracking of child scopes.
- `T*` is a borrowed reference whose owner must outlive the borrower. Symbols and LLVM objects are
  commonly referenced this way.

Prefer `std::make_unique` and `std::make_shared` to raw allocation. There is no ordinary manual
`delete` path in the compiler. Moving a `unique_ptr` into a scope or moving a diagnostic bag into a
result makes the ownership transfer explicit.

Do not replace a borrowed raw pointer with shared ownership "for safety" without identifying a
real shared lifetime. Conversely, every new raw pointer relationship needs an identifiable owner
and lifetime. If a new relationship creates a cycle, document where that cycle is broken.

LLVM values are owned by LLVM contexts, modules, functions, and builders. Ace borrows them as raw
pointers. Their local names should describe the emitted value's role, such as `value`, `address`,
`function`, or `block`, rather than encode `llvm::Value` into every identifier.

## Optionality And Results

Use `std::optional<T>` for the ordinary absence of a value. Optionality should be explicit in the
type rather than represented by a sentinel or by undocumented nullability. Test an optional before
calling `.value()`; invalid user input must not reach `bad_optional_access`.

Ace uses two diagnostic result types:

- `Diagnosed<T>` always contains a usable value and any diagnostics produced while recovering;
- `Expected<T>` may contain a value, or only diagnostics when that operation cannot continue.

Both are `[[nodiscard]]` and intentionally move-only. Collect diagnostics through
`DiagnosticBag::Collect(...)` or add them explicitly, then branch before unwrapping an absent
`Expected`. Use an error symbol or placeholder sema when later phases can safely continue; do not
invent a placeholder merely to avoid propagating a fatal result.

Prefer `std::nullopt` for an absent optional branch. Commit `c3078d0` deliberately replaced
ambiguous optional ternaries with explicit `std::nullopt` branches.

## Assertions, Diagnostics, And Exceptions

`ACE_ASSERT` and `ACE_UNREACHABLE` express compiler invariants. They are not substitutes for
diagnostics caused by invalid source. If a source program can trigger a condition, diagnose it in
the owning phase and recover or return a failed `Expected`.

After a `dynamic_cast` that must succeed because an established invariant guarantees the dynamic
type, assert the result immediately:

```cpp
auto* const traitSymbol = dynamic_cast<TraitTypeSymbol*>(symbol);
ACE_ASSERT(traitSymbol);
```

Do not assert a cast used for type probing, selecting an optional branch, or diagnostic recovery.
In those cases, test the pointer and handle the alternative explicitly.

Exceptions are not Ace's internal error-propagation model. The current `try`/`catch` sites are
boundary adapters for APIs that throw, specifically filesystem and JSON parsing. Catch those
library exceptions near the boundary and translate them into diagnostics. Do not throw exceptions
between compiler phases for expected compilation failures.

Macros are reserved for behavior that cannot be expressed as clearly by a normal function, such as
capturing the assertion's file, line, and source expression. Prefer typed functions and templates
for ordinary reusable logic.

## Casts And Runtime Type Queries

The semantic class hierarchy uses RTTI deliberately. `dynamic_cast` serves two different roles:

- probing which valid semantic form is present, where a null result selects normal control flow;
- recovering a type guaranteed by a prior invariant, where the result is asserted immediately.

Keep those roles visually distinct. Do not hide invariant casts in unchecked helpers, and do not
assert probes. Use `static_cast` for conversions whose correctness is evident from the static type
or an immediately visible invariant. Avoid C-style casts. `reinterpret_cast` is not part of the
current compiler's normal vocabulary. Existing unchecked invariant casts are cleanup candidates,
not precedent for new code.

## Construction And Data Flow

Prefer direct construction with brace initialization and return complete values from functions.
Aggregate context objects are intentionally small and passed by `const&`. Constructors establish
required object state; later binding methods are used only where compilation order prevents a
relationship from being established at construction time.

Use `std::move` when ownership or an accumulated result is intentionally transferred. Do not add
`std::move` to borrowed values or return expressions without a demonstrated need; it can inhibit
copy elision and obscure ownership.

`auto` is common when the initializer makes the type clear, for iterator-heavy expressions, smart
pointer factories, casts, and long compiler types. Spell out a type when it communicates semantic
meaning that the initializer does not, such as `TypeInfo`, `ValueKind`, `Recursiveness`, or an LLVM
integer width. Avoid using `auto` to hide ownership, optionality, or an important conversion.

## Loops, Algorithms, And Lambdas

Ace uses both explicit loops and standard algorithms. Choose the form that makes state and control
flow easiest to follow:

- algorithms fit simple transforms, searches, predicates, and traversal callbacks;
- loops fit multi-step state machines, early exits, index relationships, and mutation across
  iterations;
- range `begin(...)` and `end(...)` are used consistently, while `std::ranges` is not currently a
  project convention.

Lambdas are appropriate as short callbacks whose behavior is local to an algorithm or as stored
callables that genuinely capture construction context, such as native emitter functions. Keep
captures as narrow and lifetimes as obvious as practical.

Do not use an immediately invoked lambda merely to turn ordinary branching into a `const` variable
initializer. Do not use a lambda as disguised loop machinery. If the block has meaningful control
flow, diagnostics, several returns, or a reusable name, prefer a direct branch, an explicit loop,
or a small file-local function. This guidance captures the likely concern behind the historical
`Eliminate local lambdas` TODO without banning normal callback lambdas.

## Templates And C++20 Features

Templates are used when they preserve type safety across a real compiler family, such as typed
symbol resolution, diagnostic collection, sema traversal, or generic instantiation. Keep template
implementations in headers when required. Constrain them with `requires`, `static_assert`, or
`if constexpr` when that makes misuse fail at the API boundary.

Do not introduce template machinery solely to remove a few repeated lines. Ace currently makes
limited use of concepts and `std::span`, and no routine use of `std::variant`, `std::visit`, or
`std::ranges`. Their absence is not a ban, but a new use should simplify a concrete representation
or lifetime problem rather than serve as modernization by itself.

The syntax, sema, and symbol families currently use class hierarchies and virtual dispatch rather
than tagged `std::variant` values. Match that representation when extending an existing family;
choosing a variant for a new closed value domain would require its own concrete design argument.

Prefer enum classes and domain-specific structs over untyped integers or clusters of boolean
parameters when values represent compiler concepts. Preserve type information until an external
API, such as LLVM, requires a lower-level representation.

## Headers And Dependencies

Headers use `#pragma once`. Include the standard and project headers needed by the declarations in
the file; use forward declarations when only a pointer or reference appears and doing so keeps a
dependency boundary clear. Source files normally include their corresponding header first. Do not
rely deliberately on transitive includes; some current files do so accidentally, but that is
existing hygiene debt rather than a convention.

Include order is currently curated rather than automatically sorted. Preserve the local grouping
and avoid unrelated reordering. Put non-template implementation in `.cpp` files. File-local helpers
belong in the implementation file and should not enlarge a public header API.

The project namespace is `Ace`. Avoid broad `using namespace` directives in headers. When a source
file imports selected standard names, keep that choice local and consistent with the surrounding
file.

## Comments

Comments should explain an invariant, phase boundary, ownership exception, backend constraint, or
non-obvious reason. Do not narrate syntax that the code already states. A TODO should preserve
enough context to recover the intended problem; a question or speculative direction must be marked
as such rather than presented as settled design.

## Review Checklist

- Does every pointer form communicate its owner and required lifetime?
- Are syntax and sema objects kept immutable across transformations?
- Is optionality visible and checked before access?
- Does the result type distinguish recoverable diagnostics from fatal absence?
- Can invalid source reach an assertion, exception, unchecked cast, or optional unwrap?
- Is each `dynamic_cast` clearly either a probe or an asserted invariant?
- Does a lambda simplify local behavior, or hide ordinary control flow?
- Does `auto` preserve clarity about ownership and semantic type?
- Is a template or modern C++ facility solving a concrete recurring problem?
- Are names, comments, and APIs consistent with the compiler phase they represent?
