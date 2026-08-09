<picture>
    <img height="70" alt="Ace logo" src="/logo.svg" style="display: inline;">
</picture>

# Ace Programming Language

![License](https://img.shields.io/badge/license-MIT-ee1d48)
![Code size](https://img.shields.io/github/languages/code-size/samochreno/ace)

Ace is a statically typed, memory safe, multi paradigm programming language with consistent syntax.

## Installation

- Tested setup: macOS Apple Silicon with Homebrew and LLVM 16
- The bundled `dev`, `behavior`, and `smoke` presets currently assume Homebrew is installed under `/opt/homebrew`
- Clone the repository

```bash
git clone https://github.com/samochreno/ace
cd ace
```

- Install dependencies with Homebrew

```bash
brew install cmake ninja llvm@16 termcolor nlohmann-json
```

- Configure and build

```bash
cmake --preset dev
./scripts/ace-build build
```

## Usage

- See [example](/example)

```bash
./build/ace -oexample/build example/package.json
```

## Tests

- Runs the full behavioral suite:

```bash
ctest --preset behavior
```

- Runs only the original end-to-end smoke check: builds `ace`, compiles the bundled example, runs `./example/build/example`, and verifies the output is `0`

```bash
ctest --preset smoke
```

- To focus on one area:

```bash
cd build
ctest --output-on-failure -L scope
ctest --output-on-failure -L traits
ctest --output-on-failure -L operators
```

## License

- The project is licensed under the MIT License. See [LICENSE](/LICENSE.md) for details.
