# Runtime, Standard Library, And Emission

## Embedded Standard Library

The standard library is authored as Ace source in `std/*.ace`. CMake runs `cmake/EmbedStd.cmake` to
generate C++ data that creates in-memory `FileBuffer`s at compiler startup. This gives std source
real paths and source locations while shipping it inside the compiler binary.

This replaced hundreds of lines of source text embedded manually in `src/Std.cpp` (`fc6cd7f`). The
design goal is both readable/testable Ace source and a self-contained compiler executable.

Embedded buffers carry `SourceOrigin::Compiler`; files read from package paths carry
`SourceOrigin::User`. Ownership must never be inferred from a package name or path because users can
imitate both.

## Compiler-Owned Names

Compiler std declarations use the `__` prefix. Examples include primitive backing types, native
extern functions, reference wrappers, strong/weak pointer representations, and runtime helpers.
User declarations with that prefix are rejected centrally during declaration.

Some compiler operations are also parser-recognized internal keywords, including `__address_of`,
`__size_of`, `__deref_as`, `__copy`, `__drop`, `__type_info_ptr`, and `__vtbl_ptr`. They remain
subject to syntax/sema/type-checking phases; being internal does not make them raw emitter escapes.

The prefix is intentionally concise for std source. It is not a public compatibility namespace.
Internal names may change with compiler/runtime refactors, and no legacy aliases should be added
unless the language explicitly makes an API public.

## Native Boundary

`Natives` constructs and tracks irreducible compiler/backend functions and maps them to declared std
symbols. `Natives::Verify()` runs after declaration and before body binding, so missing or mismatched
embedded declarations become diagnostics rather than optional-access crashes.

Primitive numeric behavior is intended to be layered:

1. Ace operator syntax resolves through traits and impl functions.
2. std impl functions call compiler-owned extern declarations.
3. native emitters generate the LLVM operation.

The std declarations and impls support this model, but current operator resolution still uses
native maps directly in unary lookup and alongside trait lookup for binary operations. The boundary
is therefore transitional rather than fully enforced.

## References And Reference Counting

`std/ref.ace` and `std/rc.ace` define language-level representations and helpers for references,
strong pointers, weak pointers, dynamic strong pointers, control blocks, and vtables.

Strong pointer construction allocates value/control-block storage, copies the value, records type
information, and initializes counts. Copies and drops adjust counts through generated or std
operations. Weak locking creates a strong representation only while the control block remains
live. Dynamic strong pointers carry a vtable pointer in addition to value/control-block data.

These details are current runtime contracts, not a complete proof of the planned memory-safety
model. Initialization analysis and the final lifetime trait remain open work.

## Copy And Drop Glue

Glue generation scans concrete, non-placeholder types after body binding and initial mono
instantiation. For each eligible type it:

1. declares copy/drop function symbols and parameters through normal scope ownership;
2. binds parent pointers;
3. binds an emit-ready block produced by the type;
4. runs monomorphization discovery again because glue can reference generic behavior.

References do not receive ordinary copy/drop glue. Error and placeholder types are skipped.
Glue currently checks native copy/drop operation maps first and otherwise generates recursive
fieldwise or trivial behavior. The planned `Lifetime` trait is not yet the sole source of truth for
this decision.

Generated glue is compiler behavior represented by symbols and emittables, not hidden mutation in
arbitrary expression emitters.

## LLVM Emission

`Emitter` owns LLVM context, module, builders, function/block state, type/function maps, local
allocas, drop metadata, and vtable/type-info emission. It receives resolved, checked, lowered, and
validated semas.

The emitter may assert structural invariants such as the existence/signature of `main`, resolved
LLVM types, and established symbol categories. It must not decide which source symbol an expression
meant or whether a conversion was legal.

Current output flow:

- save sema logs;
- save unoptimized LLVM IR;
- run LLVM's O3 module pipeline;
- save optimized LLVM IR and bitcode;
- invoke external `llc` to create an object;
- invoke `clang` to link the executable.

The unconditional debug artifacts and external executable dependency are known open edges. They
should eventually become explicit compiler options or direct library integrations, but changing
them must preserve diagnostics and the tested LLVM 16 behavior.

## Runtime Verification

Backend work requires more than successful compiler construction. Tests should compile Ace input,
run the emitted executable, verify its exit status/output, and use IR checks only for behavior that
cannot be observed reliably at source level. Embedded std validation should parse and declare all
compiler-owned source so generated C++ embedding and native contracts cannot silently drift.
