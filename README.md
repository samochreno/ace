<picture>
    <img height="70" alt="Ace logo" src="/logo.svg" style="display: inline;">
</picture>

# Ace Programming Language

![License](https://img.shields.io/badge/license-MIT-ee1d48)
![Code size](https://img.shields.io/github/languages/code-size/samochreno/ace)

Ace is a statically typed, memory safe, multi paradigm programming language with consistent syntax.

## Installation

- Clone the repository

```bash
git clone --recursive https://github.com/samochreno/ace
cd ace
```

- Build the dependencies

```bash
cd dep
```

```bash
# ikalnytskyi/termcolor
cd termcolor
cmake -B build
cmake --build build
cmake --build build --target install
```

```bash
# nlohmann/json
cd nlohmann/json
cmake -B build
cmake --build build
cmake --build build --target install
cd ../..
```

```bash
# llvm/llvm-project
cd llvm-project/llvm
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --build build --target install
cd ../..
```

- Build the project

```bash
cmake -B build
cmake --build build
```

## License

- The project is licensed under the MIT License. See [LICENSE](/LICENSE.md) for details.
