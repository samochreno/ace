# Language Model

Ace describes itself as statically typed, memory safe, multi-paradigm, and syntactically
consistent. The current language combines value-oriented structs, explicit references and owned
pointers, modules and visibility, generics with constraints, and trait-based static/dynamic
dispatch.

This document records current semantics and visible direction. It is not a complete language
specification.

## Declarations And Consistent Syntax

Declarations generally place the declared name first and the declaration kind or type after it:

```ace
Point: struct {
    x: int,
    y: int,
}

distance(point: &Point): int {
    ret point.x + point.y;
}
```

`pub ::` and `pub extern ::` qualify following declarations. `pub struct` separately changes the
default visibility of a struct's members. Modules can be declared in multiple source files and
merge through partial declaration semantics. `::` separates name sections, and a leading `::`
selects global rather than local resolution.

## Values, References, And Ownership

Ace distinguishes:

- plain values such as `Point`;
- references such as `&Point`;
- strong owned pointers such as `*Point`;
- weak pointers such as `~Point`;
- dynamic strong pointers such as `*Trait` when the pointee is unsized/dynamic.

`TypeInfo` combines a type symbol with `ValueKind::L` or `ValueKind::R`. Reference binding requires
an lvalue; persistent references to rvalues are rejected during type checking. `box`, `lock`,
`unbox`, and implicit strong/weak or concrete/dynamic conversions are semantic operations, not raw
LLVM pointer casts.

The runtime representation is currently implemented by compiler-owned generic structs and
functions in `std/rc.ace`. Lifetime behavior exists through generated copy/drop glue and a
`Lifetime` trait direction, but the lifetime model is explicitly unfinished in `TODO.md`.

## Generics And Constraints

Generic parameters use brackets and are part of symbol identity:

```ace
Wrapper[T]: struct {
    value: T
}

read[T](value: &T): T where T: Readable {
    ret value.read();
}
```

Constraints are declared symbols and participate in resolution and instantiation. A `where` clause
can require multiple traits with `+`. Generic bodies are not textually copied; declared symbol trees
are instantiated with type arguments, parent links are rebound, and referenced monomorphizations
are discovered from semas.

Associated types are not implemented and should not be assumed by extensions to trait lookup.

## Traits And Dispatch

Traits declare prototypes. Impl blocks provide functions for a concrete type. `Self` names the
implementing type; `self` is the receiver. Receiver markers distinguish value and owned-pointer
calling forms.

```ace
Value: trait {
    self ::
    value(): int;
}

impl Value for int {
    self ::
    value(): int {
        ret self;
    }
}
```

Supertraits use `Trait: trait: Supertrait`. Implementing a derived trait does not silently create
the supertrait impl; whole-program validation requires it. Static dispatch resolves through trait
impl functions. Dynamic dispatch uses owned trait pointers, vtables, and only prototypes that are
dyn-dispatchable.

Trait visibility for member lookup is explicit: local trait declarations and trait `use`s determine
which impl members are considered. Ambiguous matching trait members are diagnosed rather than
selected by declaration order.

## Operators

Arithmetic, equality, and bitwise operator traits are declared in `std/op.ace`, and primitive impls
delegate to native integer/float externs. User operator semas lower the resolved function to a call.

Resolution is not yet uniformly trait-first. Unary lookup currently uses native operator maps.
Binary lookup consults both native maps and trait prototypes/impls, reports ambiguity if both match,
and validates argument convertibility on the selected function. Some operators in `Op` do not yet
have traits in `std/op.ace`. Treat removal of the native semantic path as an unfinished strict
cutover, not a current invariant.

Short-circuit boolean `&&`/`||` and logical `!` still have dedicated semantic paths. The TODO for a
logical-negation trait is direction, not current behavior. New operator work should first decide
whether it completes the trait cutover rather than adding another resolution authority.

## Visibility And Coherence

Visibility is checked across nested scopes and public interfaces. A public declaration may not leak
a less-visible type or trait through fields, parameters, returns, generics, constraints, or
supertraits.

Impl coherence is package based: inherent impls belong with their type; trait impls belong with
either their trait or type. Overlapping inherent impls and ambiguous trait members are diagnosed.

## Compiler-Owned Language Surface

Names beginning with `__` are reserved for declarations from compiler-origin source buffers.
Embedded std can declare `__Int8`, `__StrongPtr`, or `__address_of`-related helpers; user source
cannot imitate that ownership. The rule currently forbids user declarations, not references to
existing compiler internals.

Compiler internals should remain subject to normal language rules where practical. The prefix is a
namespace boundary, not permission to bypass scoping, typing, generics, traits, or diagnostics.

## Open Language Areas

The following are explicitly unfinished or undecided:

- associated types;
- the complete lifetime/default/copy/drop model;
- making `__deref_as[T]` accept values convertible to `Ptr` and removing `Ref[T]::ptr`;
- logical negation as a trait;
- safe initialization/data-flow analysis;
- dynamic cast failure semantics;
- strings;
- enums and struct-update syntax;
- mutability syntax and guarantees;
- reflection, metadata, DLLs, and shared globals;
- whether local shadowing remains allowed.

Design these from language semantics first. Existing parser tokens, partial semas, or TODO wording
may be experiments rather than final contracts.
