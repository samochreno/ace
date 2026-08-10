# Ace Agent Guide

## Design Documentation

Ace's architectural and language-design documentation lives in [`docs/`](docs/README.md).
Read the relevant documents before planning, implementing, or reviewing a compiler or language
change. Start with:

- [`docs/design-principles.md`](docs/design-principles.md) for project-wide intent;
- [`docs/compiler-pipeline.md`](docs/compiler-pipeline.md) for phase ownership;
- [`docs/nodes-and-passes.md`](docs/nodes-and-passes.md) for syntax/sema transformation rules;
- [`docs/symbols-scopes-and-resolution.md`](docs/symbols-scopes-and-resolution.md) for declarations,
  ownership, lookup, imports, traits, and generics;
- [`docs/diagnostics-and-recovery.md`](docs/diagnostics-and-recovery.md) for error handling;
- [`docs/language-model.md`](docs/language-model.md) for source-language direction;
- [`docs/runtime-stdlib-and-emission.md`](docs/runtime-stdlib-and-emission.md) for compiler internals,
  glue, the embedded standard library, and LLVM;
- [`docs/evolution.md`](docs/evolution.md) when current code and intended design appear to disagree.

Treat the docs as a map of design intent, not permission to ignore the code. Verify assumptions
against current implementations and tests. When a change intentionally alters a documented
contract, update the documentation in the same change. Do not silently turn an unfinished or
accidental behavior into a design rule.

## Build And Test

Use `./scripts/ace-build build` for builds. It serializes concurrent CMake builds for the same
build directory. Behavior tests use the same lock automatically.

Run `ctest --preset behavior` for the full suite. A compiler change is not verified merely because
Ace compiles: relevant success tests must also run the generated program and check its behavior.

## Formatting

C++ sources are formatted with the `clang-format` binary from the configured LLVM toolchain. Run
`./scripts/ace-build build --target format` to apply formatting and
`./scripts/ace-build build --target check-format` to verify it.

## C++ Code Rules

After a `dynamic_cast` that is expected to succeed by an established compiler invariant,
immediately `ACE_ASSERT` the result. Do not assert casts used for type probing, optional branching,
diagnostics, or recovery.
