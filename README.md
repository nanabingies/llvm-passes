# llvm-passes
Custom LLVM passes for program analysis and optimization.

## Overview
This repository contains LLVM passes designed for:
- Dead Function Elimination – Identifies and removes unreachable functions.
- Control Flow Analysis – Extracts and visualizes control flow graphs (CFGs).
- Symbolic Execution Enhancements – Improves program verification techniques.

# Installation

## Prerequisites
- LLVM (version 14 or later)
- CMake
- Clang

## Build Instructions
1. Clone the repository:
``` console
$ git clone https://github.com/nanabingies/llvm-passes.git
$ cd llvm-passes
```

2. Configure and build each pass
### To build DeadFunctionAnalyzer
``` console
$ mkdir build/ && cd build/
$ clang++ -S -emit-llvm ../tests/test01.c -o test01.ll
$ clang++ -fPIC -shared -std=c++17 -g `llvm-config --cxxflags --ldflags --system-libs --libs core passes` -o deadfunc.so ../src/DeadFunctionAnalyzer.cpp
$ opt -load-pass-plugin ./deadfunc.so -passes=dead-func -disable-output test01.ll
```
