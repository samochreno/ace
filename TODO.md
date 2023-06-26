# ✅ Ace To-Do List

## 💥 High Priority

- Consider splitting `Core.hpp` into multiple files, especially glue generation
- Un-nest names for example `Node::...`, `BoundNode::...`
- Refactor expression parsing
- Refactor compound assignment lowering
- Refactor emitting of variable references
- Remove primitive references
- Use `Measured` in lexer
- ❓ Remove `ACE_TRY` macros, make opening braces on the same line
- Add error messages to make debugging easier:
  - Remove second typechecking pass
  - `Compilation` parsing and verification errors
  - Parsing errors:
    - Keep track of tokens which were used in constructing a node, then if an error with a node wants to be shown, it will just call `node->CollectTokens()` and mark the error from the begining of the first token, to the end of the last token | Each node has a tokens start and end index
  - Error printing
  - ❓ Make errors just enums, not full types
- Rename `found...It`
- Add strong pointer self
- Traits:
  - Implement parsing
  - Implement symbols
  - Implement binding
  - ❓ Implement Rust's orphan rule
- Templates:
  - Implement constraints
  - When a template wants to be instantiated, check if the template has ever been instantiated, and if not, do semantic analysis on it first
  - After compilation, do semantic analysis on templates that have never been instantiated
  - ❓ Create unique signatures for template instances
  - Default template arguments
- Conversion operators with traits:
  - Implement
  -  Change `__deref_as[T](...)` to take any type convertible to `std::Pointer` and remove `std::Reference[T]::ptr(...)`
- Strings
- Make copying into unintialized variables safe (When a unintialized variable's field is dropped, it could cause unwanted behaviour &rarr; This could maybe be fixed by forcing the copy trait to also implement default trait)
- Add lifetime trait: default, copy, drop
- ] Check if there is any better way to not use external programs like `llc` and `clang`

## 🔔 Medium Priority

- Rewrite Package.json to package.ace:
  ```rs
  import std::build::*;

  package(): Package 
  {
      ret new Package {
          name: "ace",
          path_macros: [
              new PathMacro {
                  name: "$project_path",
                  value: "/home/samo/repos/ace/ace/src/",
              },
          ],
          file_paths: [
              "$project_path/**.ace",
          ],
          dependency_file_paths: [
              "$project_path/**.dll"
          ],
      };
  }
  ```
- Change `TransformExpectedVector(vec, f)` to `TransformExpected(begin, end, f)` (I think the current version of TransformExpected(...) is buggy)
- Check out LLVM attributes
- Struct update constructor syntax
- Mutability/Immutability:
  - Implement
  - Mutable references have to be taken with `mut` eg.: `mutating_function(mut mutable_expression)`
- Dont allow private types to leak in public interface
- Struct construction syntax:
  - New syntax for struct construction. The current `new Struct { ... }` and `Struct::new(...)` are too similiar
  - Add struct update constructor syntax
- Full and partial template specialization
- Add associated variables
-  Remove duplicate module filepaths: `./dawg/../dawg/` is the same as `./dawg/`
- Operator verification:
  - Operators must be public and static (this can be implemented in the parser)
  - A conversion operator must convert to the enclosing type, from a type other than the enclosing type.
  - You can have a `impl` or an `expl` operator to a type, not both
  - First parameter of `drop` has to be of type `&Self`
  - First and second parameters of `copy` have to be of type `&Self`
  - `==` reqiures a matching `!=` &larr;
  - `<=` requires a matching `>=` &larr; These will be handled by traits, eg.: `Equatable` &rarr; `operator ==(...)` and `operator !=(...)`
  - `<` requires matching `>`     &larr;

## 🥶 Low Priority

- Local variable assignment analysis (data flow analysis), control flow graph
- Metadata:
  - Export/import symbol metadata for reflection and headers
  - Metadata will be stored in exe/dll files this way:
    - Last 8 bytes of file will be u64 which stores value of metadata size in bytes
    - `[ ? bytes - Content of exe/dll ][ X bytes -  Metadata ][ 8 bytes - X ]`
- Dll package dependencies
- Dll calls
- Reflection
- Enums
- Figure out how to share globals between dlls
- ❓ Remove local variable shadowing, it causes more mistakes than usefulness

## 💡 Ideas

- Memory safe temporary references for things like Rc::...::unwrap()
- Binary operators on enums return `int`, so you cant have an arbitrary enum + safe switch:
  ```rs
  node_type: auto = NodeType::Expression;
  
  switch(node_type) # If all cases are not included, compile error.
  {
      NodeType::None | NodeType::Statement 
      { 
          ret m_dn; 
      }
      
      NodeType::Expression
      { 
          ret deez; 
      }
  }
  ```
- Inline struct:
  ```rs
  impl JSON
  {
      try_parse(value_to_parse: &String): { did_parse: bool, value: i32 } -> pub
      {
          ret new { did_parse: true, value: value_to_parse };
      }
  }
  ```
- Do block (it could be named something different, I'm not sure about the syntax either): https://www.youtube.com/watch?v=QM1iUe6IofM&ab_channel=BrianWill `41:51`
  ```rs
  start_scope: auto = do -> Scope 
  {
      scope: mut *Scope = self;
      while scope
      {
          first_name_token: auto = options.name_tokens.front();
  
          if (scope.defined_symbols_map.contains(first_name_token.string_value))
              break;
  
          scope = scope.get_parent();
      }
  
      ret scope;
  };
  ```
- Equivalent of Rust's `operator ?`:
  ```rs
  fn read_username_from_file() -> Result<String, io::Error> {
      
      // If this returns an error, operator ? automatically returns the error.
      let mut f = File::open("hello.txt")?; 
  
      let mut s = String::new();
      f.read_to_string(&mut s)?;
  
      Ok(s)
  }
  ```
- Loops:
  ```rs
  main(): void -> pub
  {
      array_1: auto = List::Array[i32]::new({ 10, 51, 14, 38 });
      array_2: auto = List::Array[i32]::new({ 50, 10, 51, 14, 38 });
  
      for_each(array_1[interval<begin, end)], item =>
      {
      });
  
      for_each(begin(array_1), end(array_1), item => 
      {
      });
  
      for_each(Zip::new(array_1.beg(), array_1.end(), array_2.beg() + 1, array_2.end()), pair => 
      {
          assert pair.First == pair.Second;
      });
  }
  ```
- Import and export functions (Export will disable name mangling):
  ```rs
  Program: struct
  {
      import_func(): void -> pub import;
      
      export_func(): void -> pub export
      {
          # Do something ...
          import_func();
      }
  }
  ```
- Equivalent of Swift's `guard` statement:
  ```swift
  var i = 2
  
  while (i <= 10) {
  
    // Guard condition to check the even number.
    guard i % 2 == 0 else {
  
       i = i + 1
      continue
    }
  
    print(i)
    i = i + 1
  } 
  ```
- Equivalent of Swift's wildcard:
  ```swift
  func allocateUTF8String(
    _ string: String, // Wildcard allows not typing out the parameter name when calling the function.
    nullTerminated: Bool = false
  ) -> (UnsafePointer<UInt8>, Int) {
    var string = string
    return string.withUTF8 { utf8 in
      let capacity = utf8.count + (nullTerminated ? 1 : 0)
      let ptr = UnsafeMutablePointer<UInt8>.allocate(
        capacity: capacity
      )
      if let baseAddress = utf8.baseAddress {
        ptr.initialize(from: baseAddress, count: utf8.count)
      }

      if nullTerminated {
        ptr[utf8.count] = 0
      }

      return (UnsafePointer<UInt8>(ptr), utf8.count)
    }
  }
  ```
- Equivalent of Rust's `Self` type
- `Ret` type:
  ```rs
  func(): std::Optional[int] {
      ret Ret::new(5);
  }
  ```
