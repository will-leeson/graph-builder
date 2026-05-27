# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

This project converts programs and logical formulas into graph representations for use with graph neural networks. It has three independent sub-projects targeting different source languages.

## Sub-projects

### C graph builder (`src/c/`)

Parses C programs via the Clang AST and emits program graphs combining AST structure, control flow, call graph, and data-dependency edges.

**Dependencies:** LLVM 11, Clang 11 (`libclang-11-dev`, `clang-tools-11`)

**Build:**
```bash
mkdir build && cd build
cmake ..
make
# Optional: make install
```

**Run:**
```bash
./build/src/c/app/c-graph-builder [file.c] [-flags]
./build/src/c/app/c-graph-builder -h   # list available flags
```

Flags: `--ast`, `--icfg`, `--call`, `--data`, `--chain-length N`, `--output-file <path>`, `--print`

### Java graph builder (`src/java/`)

Parses Java bytecode via SootUp and emits block-level CFG, statement-level CFG, CDG, expression graphs, and call graphs.

**Build:**
```bash
cd src/java && mvn package
```

**Run:** Entry point is `src/java/src/main/java/com/lesslab/ProgramGraph/App.java`

**Test:**
```bash
cd src/java && mvn test
```

### SMT graph builder (`src/smt/`)

Converts SMT-LIB2 formulas to graph tensors using pySMT, outputting numpy `.npz` files or PyTorch Geometric `HeteroData` objects.

**Python dependencies:** `pysmt`, `networkx`, `numpy`, `torch`, `torch_geometric`

**Homogeneous graph (treewalker.py):**
```bash
python3 src/smt/treewalker.py --file formula.smt2 --out graph.npz [--dot]
```

**Heterogeneous graph (heterotreewalker.py):**
```bash
python3 src/smt/heterotreewalker.py --input <in_folder> --output <out_folder> [--timeout 30] [--max_nodes 150000] [--workers N]
```

Walks `in_folder` recursively, converts every `.smt2`/`.smt` file in parallel, and writes `.pt` files mirroring the directory structure under `out_folder`. Skips already-converted files. Errors are logged to `conversion_errors.log`.

**Visualize a saved `.pt` graph:**
```bash
python3 src/smt/visualize.py path/to/graph.pt
```

## Architecture

### C component

`GraphBuilderConsumer` in `src/c/graph-builder/graph-builder.h` is the top-level orchestrator. It holds four `RecursiveASTVisitor` instances and runs them in dependency order:

1. **`ASTBuilder`** — always runs first; establishes the node/ID mapping that all other builders rely on.
2. **`ICFGBuilder`** — runs when `--icfg` or `--data` is requested; computes gen/kill sets and references needed by data flow.
3. **`DataFlowBuilder`** — runs after ICFG; consumes the gen/kill and reference maps to perform reaching-definitions analysis.
4. **`CallGraphBuilder`** — independent of the others; links call sites to callees.

Each builder owns a `graph` instance (defined in `src/c/utils/utils.h`). After traversal, the consumer calls `graph::mergeGraph()` to combine all four into one output graph. Edge type is encoded as an integer attribute (`attr` constructor parameter: 0=AST, 1=ICFG, 2=call, 3=data).

The `graph` class serializes node and edge lists to parallel integer vectors (`nodesSerial`, `outEdgesSerial`, `inEdgesSerial`) and can write to a file or print to stdout.

### Java component

`GraphBuilder` in `src/java/…/GraphBuilder.java` takes a classpath and boolean flags for each graph type. It uses two SootUp views: `localView` (project classes only) and `globalView` (project + `rt.jar`) for type hierarchy and call resolution. `GraphStmtVisitor` and `GraphValueVisitor` walk the Jimple IR to build expression-level edges. Output is JSON with parallel `nodes`, `outEdges`, `inEdges`, `edgeTypes`, and `location` arrays.

### SMT component

Both walkers extend pySMT's `TreeWalker`. `treewalker.py` (`GraphBuilder`) produces a homogeneous graph where every SMT operator is a node, symbols and constants share identity nodes (two extra synthetic node types beyond pySMT's `ALL_TYPES`), and edge attribute 0 = AST parent/child, 1 = def-use link. Output is a compressed `.npz` with `nodes`, `edges`, `edge_attr` arrays.

`heterotreewalker.py` (`SMT_HGT_Builder`) groups operators into semantic node types (`logic`, `arithmetic`, `bitvector`, `memory`, `string`, `use`, `definition`) and produces a PyTorch Geometric `HeteroData` object with typed nodes and `(src_type, "child"|"identity", dst_type)` edge relations. Symbol use-sites become `use` nodes; shared identity nodes for variables and constants become `definition` nodes. The main entry point runs batch parallel processing with per-file timeouts and a node-count circuit breaker.
