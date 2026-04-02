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

- Install dependencies with Homebrew

```bash
brew install cmake ninja llvm@16 termcolor nlohmann-json
```

- Configure and build

```bash
cmake --preset dev
cmake --build --preset dev
```

## Usage

- See [example](/example)

```bash
./build/ace -oexample/build example/package.json
```

## Smoke Test

```bash
ctest --preset smoke
```

## License

- The project is licensed under the MIT License. See [LICENSE](/LICENSE.md) for details.
