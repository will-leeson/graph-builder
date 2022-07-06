# graph-builder

This project is used to generate program graphs from C programs. These graphs are based on the programs AST which is then annotated with control and data flow edges.

## Dependencies
This project relies on the Clang-AST. In order to build it, you need to install
the following dependencies:

* `llvm-11`
* `clang-11`
* `clang-tools-11`
* `libclang-11-dev`

In most cases, these can be installed using your OS's package manager, e.g. `sudo apt-get install <Package>`

## Build
To build this project, clone this repository and follow these steps:
1. `cd graph-builder`
2. `mkdir build`
3. `cd build`
4. `cmake ..`
5. `make`
6. `make install` (optional)

## Usage
The C language graph builder can be invoked from the home directory as such `./build/src/c/app/c-graph-builder [file.c] [-flags]`. If you run step 6, it can be invoked simply via `c-graph-builder [file.c] [-flags]`

To see what graphs can be built and options, run `c-graph-builder -h`