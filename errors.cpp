#if 0

error: ambiguous symbol reference
--> src/main.ace:3:1
|         call(10)
|         ^^^^
note: candidate symbol declaration
--> src/main.ace:15:1
|         call(x: int)
|         ^^^^
note: candidate symbol declaration
--> src/main.ace:28:1
| call(x: int, y: int)
| ^^^^ 

error: scope access of a local variable
--> src/main.ace:3:1
|         label::pes
|         ^^^^
note: local variable declaration
--> src/main.ace:15:1
|   label: int = 0;
|   ^^^^^

error: inaccessible field
--> src/main.ace:3:1
|     vec.x = 15;
|         ^
note: field declaration
--> src/main.ace:15:1
|     x: int,
|     ^

error: missing fields `x`, `y` and `z`
--> src/main.ace:15:1
|     vec: auto = Vector2 {  };
|                 ^^^^^^^
note: struct declaration
--> src/main.ace:1:1
| Vector2: struct -> pub {
| ^^^^^^^

error: symbol is not a function
--> src/main.ace:52:10
|       vec.x();
|           ^
note: symbol declaration
--> src/main.ace:5:5
|     x: int -> pub,
|     ^

error: not an instance symbol
--> src/main.ace:15:8
|         self.fn();
|              ^^
note: symbol definition
--> src/main.ace:5:5
|     fn(): int {
|     ^^

error: field initialized more than once
--> src/main.ace:90:16
|           Vector2 { x: 10, x: 10, y: 15 },
|                            ^
note: previous initialization
--> src/main.ace:67:7
|           Vector2 { x: 10, x: 10, y: 15 },
|                     ^

error: `Vector2` has no field named `z`
--> src/main.ace:90:16
|           Vector2 { z: 10, x: 10, y: 15 },
|                     ^
note: `Vector2` declaration
--> src/main.ace:1:1
| Vector2: struct -> pub {
| ^^^^^^^

error: undefined reference to operator `-` for type `int`
--> src/main.ace:3:1
|        -10
|        ^

error: undefined reference to operator `-` for types `int` and `float`
--> src/main.ace:3:1
|        10 + 7.5
|           ^

#endif
