# Ace To-Do List

## 🟢 Refactor

- Make `llvm::Value*` variable names consistent
- Refer to `Self` and `self` consistently (`Self` is sometimes `SelfType`)
- Cache `Scope::CollectSymbols` or just generally cache in `Scope` for performance
- Rename `ControlFlowNode` to `ControlFlowInstruction` or something, node implies its a tree structure but really its linear
- Make diagnostics take symbols, syntaxes, etc. instead of source locations, it allows focusing on the important code in the function
- ❓ Eliminate local lambdas
- ❓ String interning
- ❓ Double underscore all public names that shouldnt be used: `__StrongPtr`, `__Ref`

## 🔴 High Priority

- Supertraits
- Remove redundant undeclared symbol error for unimplemented function: When resolving associated functions, check if the type implements a trait with a function of that name, then just 'trust' it is implemented
- Replace operators with traits
- Change `__deref_as[T](...)` to take any type convertible to `std::Ptr` and remove `std::Ref[T]::ptr(...)`
- Make copying into unintialized variables safe (When a unintialized variable's field is dropped, it could cause unwanted behaviour &rarr; possibly fixed by lifetime trait)
- Lifetime trait: default, copy, drop
- Fix that glue doesnt bind on private types
- Dynamic casting (how to handle failed conversions?)
- Strings

## 🟡 Medium Priority

- Forbid taking reference of a r-value
- Add self type signature to member access resolution error message
- Diagnostics when redeclared type parameter
- Forbid constructing of non-pub struct
- Struct update constructor syntax
- Add diagnosis for leaking private types in public interface
- Add source location to unknown files like `.cpp`
- Control flow flexibility improvement:
  - `IEmittable` could hold `ControlFlowGraph`, so it could verify control flow
  - Possibly implement `ControlFlowNode`s for `LLVM IR` instructions (check `llvm/IR/Instruction.def`)
- Check if there is any a better way than to depend on external executables like `llc` and `clang`
- Remove duplicate module filepaths in package: `./dawg/../dawg/` is the same as `./dawg/`
- Enums
- Mutability/Immutability:
  - Mutable references have to be taken with `mut` eg.: `mutating_function(mut mutable_expression)`
- Unit testing (Test that `CreateTokenKindString` handles all possible values, test `Keyword.cpp`)

## ⚪ Low Priority

- Local variable assignment analysis (data flow analysis), control flow graph
- Metadata:
  - Export/import symbol metadata for reflection and headers
  - Metadata will be stored in exe/dll files this way:
    - Last 8 bytes of file will be u64 which stores value of metadata size in bytes
    - `[ ? bytes - Content of exe/dll ][ X bytes -  Metadata ][ 8 bytes - X ]`
- Dll package dependencies
- Dll calls
- Reflection
- Figure out how to share globals between dlls
- ❓ Remove local variable shadowing, it causes more mistakes than usefulness
