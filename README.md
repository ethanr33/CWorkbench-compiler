# CWorkbench Compiler

CWorkbench is a C compiler written in C++. It compiles a subset of the C language into x86-64 NASM assembly. This repository contains only the compiler core, the web application is in a separate project. The goal of the project as a whole is to integrate this compiler with a web application to let users test the compiled code and run their C online.

## Features

- Handwritten lexer and LL(1) parser
- AST construction and traversal
- NASM-supported x86-64 code generation

# Supported C features

Currently, the compiler only supports C programs with a simple main function which returns an integer or sum of integers. This will be expanded upon in the future.

Example program:

```c
    int main() {
        return 0;
    }
```

## Directory Structure

```

cworkstation-compiler/
├── codegen/         # NASM assembly code generation
├── lexer/           # Tokenizer implementation
├── parser/          # Grammar definitions, LL(1) parser and AST generator
├── tests/           # Tests are located here
│   └── integration/ # Integration tests
├── tmp/             # Temporary files generated during building
├── tools/           # Various tools for debugging
├── main.cpp         
├── CMakeLists.txt
└── README.md

````

## Building

### Requirements

- C++23 or later
- CMake 3.31+
- NASM (for assembling generated output)

### Build Instructions

For Linux:

```bash
./build.sh
````

## Usage

For Linux:

```bash
./run.sh <file path>
```

This will compile the input C file and emit NASM-compatible assembly to `out.asm`.

To compile without running the generated executable: 

```bash
./run.sh <file path> --compile-only
```


