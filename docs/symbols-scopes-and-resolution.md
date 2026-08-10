# Symbols, Scopes, And Resolution

## Scope Tree And Ownership

A `Scope` owns its declared symbols in a map from name to vectors of `unique_ptr<ISymbol>`. Returned
`ISymbol*` values are borrowed pointers into this storage. A symbol chooses its destination through
`GetScope()`; body-scoped symbols expose a child body scope whose parent is the owning scope.

Scopes form a lifetime graph:

- child scopes hold their parent with `shared_ptr`;
- parents track children with `weak_ptr`;
- syntax and body-scoped symbols can keep child scopes alive;
- `GlobalScope::~GlobalScope()` recursively clears symbols and parent links to break the graph.

Removing a symbol destroys it immediately and invalidates borrowed pointers. `RemoveSymbol()` has
no production call sites and should not become ordinary mutation without a lifetime audit.

## Source Declaration Flow

Parsing creates the syntax and scope trees first. `Application::CollectSyntaxes()` flattens the
owned ASTs into borrowed pointers. `CreateAndDeclareSymbols()` filters `IDeclSyntax` objects and
sorts them by `DeclOrder` and `GetDeclSuborder()`.

Source declarations enter through `Scope::DeclareSymbol(const IDecl*)`:

1. A partial declaration, currently a module, may find an existing symbol and extend it through
   `ContinueCreatingSymbol()`.
2. Other declarations call `CreateSymbol()` and forward the owned result to the canonical owning
   overload.

Source-only rules belong on this path and must account for the partial-declaration early return.
They must also distinguish explicit source declarations from parser-synthesized declarations such
as `TypeReimportSyntax`.

## Canonical Owned Declaration

`Scope::DeclareSymbol(unique_ptr<TSymbol>)` is the ownership boundary for source and generated
symbols. It:

- locates the symbol's destination scope;
- checks same-name/generic/self-type redeclarations;
- reports conflicts and returns the surviving symbol;
- inserts successful declarations into scope storage;
- notifies that scope's `GenericInstantiator`.

Callers must use the returned pointer. A conflicting candidate may be destroyed during recovery.
Compiler-created built-ins, native symbols, generic instances, glue functions and parameters,
temporaries, and labels use this same owning path.

## Parent Binding

Parameters and type parameters keep borrowed links to their owning callable or generic symbol.
`BindSymbolParents()` runs after initial declaration and after generated body symbols are
instantiated. Glue generation also binds parents after declaring generated function parameters.

These links answer semantic ownership questions without creating shared-ownership cycles.

## Names And Modules

`SymbolName` is a sequence of sections with optional generic arguments and an explicit local/global
resolution mode. Modules are partial declarations: repeated module declarations share the module
symbol/body scope, but their access modifiers must agree.

Visibility is scope-relative, not merely a `pub` boolean. Public-interface diagnosis computes the
effective visibility scope and rejects private types or traits exposed through functions, fields,
globals, constraints, generic arguments, or supertraits.

Package boundaries also define coherence:

- an inherent impl must be in the package that owns its type;
- a trait impl must be in the package that owns either the trait or the type.

## Instance Member Resolution

Resolution distinguishes inherent and trait sources.

1. The concrete/dereferenced self type and applicable inherent impl scopes are searched first.
2. Trait lookup gathers traits visible to the containing module, including explicit `use` symbols.
3. Applicable trait impls are filtered to impl bodies that contain the requested member.
4. No match falls through to constrained-trait prototypes where appropriate.
5. Multiple matching impl members produce an ambiguity diagnostic.

The implementation refactor in `45ef9dc` is the current direction: concrete instance members are
resolved from matching impl bodies, not synthetic trait scopes. Do not restore a second legacy
lookup route. The remaining `CollectImplOfFor` TODO comments indicate cleanup is unfinished, not
that the model should be bypassed.

`use` is currently specifically tied to bringing a root trait into trait-member lookup. Do not
document or implement it as a general namespace import without a separate language decision.

## Traits, Prototypes, And Impls

A trait owns a prototype scope. A trait impl owns a body scope containing implementing functions.
Whole-program trait diagnosis verifies that every prototype has a matching function, signatures
and type-parameter counts agree, impl constraints are not stricter, and no extra functions appear.
Supertrait diagnosis separately requires impls of inherited traits.

`Self` is a type-level symbol; `self` is the receiver parameter/value. Their recent naming split is
intentional and should remain visible in APIs.

## Generic Instantiation

Each scope has a `GenericInstantiator`. Root generics are registered when declared. Instantiation:

1. verifies type-argument count, sizedness, and constraints;
2. creates and declares an owned instance;
3. recursively instantiates symbols and child scopes from the root body;
4. binds parent links in the instantiated body;
5. records placeholder dependencies collected from root function semas;
6. materializes referenced monomorphizations until no new instances remain.

Generic identity is rooted in declared symbols and type arguments, not textual substitution.
Generated instances must continue using ordinary scope ownership and resolution invariants.
