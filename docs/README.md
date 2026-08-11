# Ace Design Map

This directory records the intent behind Ace's compiler architecture and language design. It is
meant to help a contributor reason in the same model as the rest of the project before changing
code.

The documentation is evidence-backed. It draws from the current implementation, behavior tests,
the example and embedded standard library, TODOs, and refactor history. It deliberately separates:

- **Principles**: repeated design choices that should guide new work.
- **Current contracts**: invariants enforced by the present compiler.
- **Direction**: intended designs that are visible but not complete.
- **Open edges**: places where code, history, and TODOs do not establish a final answer.

Do not elevate a workaround, an old implementation, or a TODO hypothesis into language doctrine.
When evidence conflicts, preserve the ambiguity and revisit the design with Samuel.

## Documents

- [Design principles](design-principles.md): the values that connect the compiler and language.
- [Compiler pipeline](compiler-pipeline.md): orchestration, phase boundaries, and stage invariants.
- [Nodes and passes](nodes-and-passes.md): syntax, semas, immutable transformations, control flow,
  and monomorphization collection.
- [Symbols, scopes, and resolution](symbols-scopes-and-resolution.md): declarations, ownership,
  lookup, modules, trait imports, impls, and generic instances.
- [Diagnostics and recovery](diagnostics-and-recovery.md): recoverable versus fatal operations,
  placeholders, assertions, and diagnostic ownership.
- [C++ style and idioms](cpp-style.md): names, ownership, constness, results, casts, lambdas,
  templates, and implementation-language boundaries.
- [Language model](language-model.md): the source-language ideas expressed by Ace today.
- [Runtime, stdlib, and emission](runtime-stdlib-and-emission.md): embedded compiler sources,
  internal names, pointer runtime, glue, natives, and LLVM output.
- [Evolution](evolution.md): architectural refactors, the intent they reveal, and unfinished areas.

## Using The Map For A Feature

Before implementing a feature, answer these questions:

1. What source-language rule is being added or changed?
2. Which phase first has enough information to enforce it?
3. What invariant will every later phase be allowed to assume?
4. Is the operation represented explicitly in syntax and sema, or is it generated/lowered?
5. Which scope owns every new symbol, and what owns every object referenced by a raw pointer?
6. Is failure recoverable? If so, what error symbol or placeholder preserves the pipeline?
7. Does the feature use ordinary language machinery, or introduce a compiler-only bypass?
8. Which success, failure, and runtime behavior demonstrate the intended semantics?
9. Does the change complete a direction, or accidentally establish a new one?

For reviews, reverse the questions: find decisions made in the wrong phase, mutable replacement of
immutable trees, hidden ownership, duplicated resolution paths, backend semantic decisions, and
tests that compile without running when runtime behavior matters.
