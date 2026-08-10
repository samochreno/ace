# Diagnostics And Recovery

## Result Types Encode Recovery

Ace uses two result wrappers with different contracts:

- `Diagnosed<T>` always contains a value plus diagnostics. The operation recovered sufficiently to
  continue.
- `Expected<T>` may contain a value plus diagnostics or may contain only diagnostics. Absence is a
  fatal result for that operation.

`DiagnosticBag::Collect(...)` merges diagnostics and returns either the diagnosed value, an
optional expected value, or a success boolean. This keeps propagation explicit and prevents
diagnostics from becoming ambient exceptions.

Use `Diagnosed<void>` for reusable validations that can report errors without producing a value.
Use `CreateXError(...) -> DiagnosticGroup` to construct one concrete diagnostic group. Use
`DiagnoseX(...) -> Diagnosed<void>` when a check is reusable or combines several groups.

## Recovery Objects

The compiler deliberately continues after many user errors:

- unresolved declarations can use compilation-owned error symbols;
- invalid conversions can return `ConversionPlaceholderExprSema` with an error type;
- declaration conflicts return the surviving symbol;
- optional resolution results stop only the dependent path.

A recovery object must satisfy later structural invariants without pretending the program is
valid. It should suppress cascades caused by the same root error, but not unrelated diagnostics.
Emission is gated on the global diagnostic bag, so error placeholders must never reach LLVM as
valid semantics.

## Fatal Boundaries

Parsing a file may fail to produce an AST. Name resolution may fail to produce a required symbol.
CLI/package parsing may fail to produce a compilation. These use `Expected` and require the caller
to branch before accessing a value.

Never use `.value()` or unwrap merely because invalid input is assumed to have been diagnosed
elsewhere. If recovery is possible, return a placeholder. If it is not, propagate an absent
`Expected` value. A user program must not cause `bad_optional_access`, an assertion, or an LLVM
crash.

## Diagnostic Ownership By Phase

- Parsing diagnostics describe tokens, grammar, and source structure.
- Binding diagnostics describe declarations, names, symbols, access, and member/operator lookup.
- Type-checking diagnostics describe values, conversions, assignments, calls, and return types.
- Generic-instantiation diagnostics describe type arguments and constraints.
- Diagnosis diagnostics describe whole-program relationships such as impl completeness, orphans,
  visibility leaks, layout cycles, and control flow.
- Emitting diagnostics describe backend/output failures, not language validity.

Put a diagnostic where the violated rule is established. The convenience of a source location or
available helper is not enough reason to move a type rule into `CreateSema()` or a binding rule into
emission.

## Assertions

`ACE_ASSERT` documents an invariant that valid or invalid source should already satisfy. After a
`dynamic_cast` expected to succeed because of an established invariant, assert immediately. Do not
assert casts used for probing optional types, choosing a semantic branch, or recovering from user
errors.

An assertion failure during a test case usually means either:

- an earlier phase failed to establish/document an invariant; or
- a recoverable source error was incorrectly treated as impossible.

Fix the phase boundary or recovery path rather than weakening assertions indiscriminately.

## Source Locations And Messages

Source locations travel with syntax, identifiers, symbols, and semas. Prefer passing the semantic
object responsible for a diagnostic when that lets the diagnostic helper choose the right name and
location. User-facing messages should use display names, not internal signatures or compiler-only
identifiers.

Diagnostic groups may include a primary error and notes pointing to declarations or previous
definitions. Avoid duplicate downstream errors when one missing declaration or impl already
explains the failure.

## Testing Diagnostics

Failure tests should assert the meaningful message and, when path distinctions matter, the relevant
location. A substring appearing once does not prove that a second declaration path was diagnosed;
tests should identify the declaration or use count-aware checks when duplication itself is the bug.
