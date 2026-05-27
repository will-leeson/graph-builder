import os, sys
import time
import signal
import torch
import argparse, traceback
from concurrent.futures import ProcessPoolExecutor, as_completed
from pysmt import operators as ops
from torch_geometric.data import HeteroData
from pysmt.walkers.tree import TreeWalker
import re
import io
import traceback
from pysmt.shortcuts import reset_env
from pysmt.smtlib.parser import SmtLibParser

sys.setrecursionlimit(20000)

# --- Timeout Configuration ---
class TimeoutException(Exception): pass

def timeout_handler(signum, frame):
    raise TimeoutException()

ID_VARIABLE = 66
ID_CONSTANT = 67
class SMT_HGT_Builder(TreeWalker):
    def __init__(self, env=None, max_nodes=100000):
        super().__init__(env)
        self.max_nodes = max_nodes
        self.node_count = 0

        self.type_map = {}
        self._setup_type_map()

        self.node_features = {
            "logic": [], "arithmetic": [], "bitvector": [],
            "memory": [], "string": [], "definition": [], "use": []
        }

        self.edge_indices = {}
        self.parent_stack = []
        self.var_map = {}
        self.const_map = {}

    def _setup_type_map(self):
        groups = {
            "logic": ops.BOOL_OPERATORS | ops.RELATIONS,
            "arithmetic": ops.IRA_OPERATORS,
            "bitvector": ops.BV_OPERATORS,
            "memory": ops.ARRAY_OPERATORS,
            "string": ops.STR_OPERATORS
        }
        for node_type, op_set in groups.items():
            for op in op_set:
                self.type_map[op] = node_type

    def get_node_type(self, formula):
        return self.type_map.get(formula.node_type(), "logic")

    def add_node(self, node_type, feature):
        # --- Circuit Breaker ---
        self.node_count += 1
        if self.node_count > self.max_nodes:
            raise MemoryError(f"Graph exceeded max node limit of {self.max_nodes}")

        idx = len(self.node_features[node_type])
        self.node_features[node_type].append(feature)
        return idx

    def add_hetero_edge(self, src_type, dst_type, src_idx, dst_idx):
        rel = (src_type, "child", dst_type)
        if rel not in self.edge_indices:
            self.edge_indices[rel] = [[], []]
        self.edge_indices[rel][0].append(src_idx)
        self.edge_indices[rel][1].append(dst_idx)

    def walk_nary(self, formula):
        this_type = self.get_node_type(formula)
        this_idx = self.add_node(this_type, formula.node_type())

        if self.parent_stack:
            p_type, p_idx = self.parent_stack[-1]
            self.add_hetero_edge(p_type, this_type, p_idx, this_idx)

        self.parent_stack.append((this_type, this_idx))
        for arg in formula.args():
            self.walk(arg)
        self.parent_stack.pop()

    def walk_symbol(self, formula):
        this_idx = self.add_node("use", formula.node_type())
        if self.parent_stack:
            p_type, p_idx = self.parent_stack[-1]
            self.add_hetero_edge(p_type, "use", p_idx, this_idx)

        name = formula.symbol_name()
        if name not in self.var_map:
            self.var_map[name] = self.add_node("definition", ID_VARIABLE)

        var_idx = self.var_map[name]
        rel = ("definition", "identity", "use")
        if rel not in self.edge_indices: self.edge_indices[rel] = [[], []]
        self.edge_indices[rel][0].append(var_idx)
        self.edge_indices[rel][1].append(this_idx)

    def walk_constant(self, formula):
        this_idx = self.add_node("logic", formula.node_type())
        if self.parent_stack:
            p_type, p_idx = self.parent_stack[-1]
            self.add_hetero_edge(p_type, "logic", p_idx, this_idx)

        c_repr = str(formula)
        if c_repr not in self.const_map:
            self.const_map[c_repr] = self.add_node("definition", ID_CONSTANT)

        const_idx = self.const_map[c_repr]
        rel = ("definition", "identity", "logic")
        if rel not in self.edge_indices: self.edge_indices[rel] = [[], []]
        self.edge_indices[rel][0].append(const_idx)
        self.edge_indices[rel][1].append(this_idx)

    # N-ary maps
    def walk_and(self, f): return self.walk_nary(f)
    def walk_or(self, f): return self.walk_nary(f)
    def walk_plus(self, f): return self.walk_nary(f)
    def walk_times(self, f): return self.walk_nary(f)
    def walk_div(self, f): return self.walk_nary(f)
    def walk_pow(self, f): return self.walk_nary(f)
    def walk_iff(self, f): return self.walk_nary(f)
    def walk_implies(self, f): return self.walk_nary(f)
    def walk_minus(self, f): return self.walk_nary(f)
    def walk_equals(self, f): return self.walk_nary(f)
    def walk_le(self, f): return self.walk_nary(f)
    def walk_lt(self, f): return self.walk_nary(f)
    def walk_bv_xor(self, f): return self.walk_nary(f)
    def walk_bv_concat(self, f): return self.walk_nary(f)
    def walk_bv_udiv(self, f): return self.walk_nary(f)
    def walk_bv_urem(self, f): return self.walk_nary(f)
    def walk_bv_sdiv(self, f): return self.walk_nary(f)
    def walk_bv_srem(self, f): return self.walk_nary(f)
    def walk_bv_sle(self, f): return self.walk_nary(f)
    def walk_bv_slt(self, f): return self.walk_nary(f)
    def walk_bv_ule(self, f): return self.walk_nary(f)
    def walk_bv_ult(self, f): return self.walk_nary(f)
    def walk_bv_lshl(self, f): return self.walk_nary(f)
    def walk_bv_lshr(self, f): return self.walk_nary(f)
    def walk_bv_ashr(self, f): return self.walk_nary(f)
    def walk_bv_comp(self, f): return self.walk_nary(f)
    def walk_bv_and(self, f): return self.walk_nary(f)
    def walk_bv_or(self, f): return self.walk_nary(f)
    def walk_bv_not(self, f): return self.walk_nary(f)
    def walk_bv_add(self, f): return self.walk_nary(f)
    def walk_bv_mul(self, f): return self.walk_nary(f)
    def walk_bv_sub(self, f): return self.walk_nary(f)
    def walk_bv_constant(self, f): return self.walk_constant(f)
    def walk_int_constant(self, f): return self.walk_constant(f)
    def walk_real_constant(self, f): return self.walk_constant(f)
    def walk_bool_constant(self, f): return self.walk_constant(f)
    def walk_algebraic_constant(self, f): return self.walk_constant(f)
    def walk_not(self, f): return self.walk_nary(f)
    def walk_function(self, f): return self.walk_nary(f)
    def walk_bv_extract(self, f): return self.walk_nary(f)
    def walk_bv_neg(self, f): return self.walk_nary(f)
    def walk_bv_ror(self, f): return self.walk_nary(f)
    def walk_bv_rol(self, f): return self.walk_nary(f)
    def walk_bv_zext(self, f): return self.walk_nary(f)
    def walk_bv_sext(self, f): return self.walk_nary(f)
    def walk_ite(self, f): return self.walk_nary(f)
    def walk_forall(self, f): return self.walk_nary(f)
    def walk_exists(self, f): return self.walk_nary(f)
    def walk_toreal(self, f): return self.walk_nary(f)
    def walk_str_constant(self, f): return self.walk_nary(f)
    def walk_str_length(self,f): return self.walk_nary(f)
    def walk_str_charat(self,f, **k): return self.walk_nary(f)
    def walk_str_concat(self,f, **k): return self.walk_nary(f)
    def walk_str_contains(self,f, **k): return self.walk_nary(f)
    def walk_str_indexof(self,f, **k): return self.walk_nary(f)
    def walk_str_replace(self,f, **k): return self.walk_nary(f)
    def walk_str_substr(self,f, **k): return self.walk_nary(f)
    def walk_str_prefixof(self,f, **k): return self.walk_nary(f)
    def walk_str_suffixof(self,f, **k): return self.walk_nary(f)
    def walk_str_to_int(self,f, **k): return self.walk_nary(f)
    def walk_int_to_str(self,f, **k): return self.walk_nary(f)
    def walk_array_select(self, f): return self.walk_nary(f)
    def walk_array_store(self, f): return self.walk_nary(f)
    def walk_array_value(self, f): return self.walk_nary(f)
    def walk_bv_tonatural(self, f): return self.walk_nary(f)

    def to_pyg(self):
        data = HeteroData()
        for node_type, features in self.node_features.items():
            if features:
                data[node_type].x = torch.tensor(features, dtype=torch.long).view(-1, 1)
        for (src, rel, dst), indices in self.edge_indices.items():
            data[src, rel, dst].edge_index = torch.tensor(indices, dtype=torch.long)
        return data

def sanitize_and_parse(file_path):
    with open(file_path, 'r') as f:
        lines = f.readlines()

    # ESBMC's dump wraps lines at exactly 1023 characters, splitting tokens
    # mid-word. Keep joining as long as the last consumed physical line was
    # exactly 1023 chars (meaning it was a wrapped continuation).
    joined = []
    i = 0
    while i < len(lines):
        raw = lines[i].rstrip('\n')
        buf = raw
        while len(raw) == 1023 and i + 1 < len(lines):
            i += 1
            raw = lines[i].rstrip('\n')
            buf += raw
        joined.append(buf + '\n')
        i += 1

    # Strip ESBMC comment lines consisting entirely of '#' characters.
    # These are not valid SMT-LIB. '#b'/'#x' literals are safe — they
    # contain non-'#' characters so won't match.
    joined = [l for l in joined if not all(c == '#' for c in l.strip()) or not l.strip()]
    content = ''.join(joined)

    parser = SmtLibParser()
    script = parser.get_script(io.StringIO(content))
    return script.get_last_formula()


def worker_task(file_info):
    file_path, out_path, timeout, max_nodes = file_info

    def local_timeout_handler(signum, frame): raise TimeoutException()
    signal.signal(signal.SIGALRM, local_timeout_handler)
    signal.alarm(timeout)

    try:
        reset_env()

        formula = sanitize_and_parse(file_path)
        gb = SMT_HGT_Builder(max_nodes=max_nodes)
        gb.walk(formula)
        data = gb.to_pyg()
        torch.save(data, out_path)
        return "success"

    except TimeoutException:
        return "timeout"
    except MemoryError:
        return "oversized"
    except Exception:
        # DO NOT use str(e) or f"{e}" here.
        # It triggers the pySMT serializer which is currently crashing.
        # Just return the type of error and the filename.
        err_type = traceback.format_exc().splitlines()[-1]
        return f"error: {err_type}"
    finally:
        signal.alarm(0)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', type=str, required=True)
    parser.add_argument('--output', type=str, required=True)
    parser.add_argument('--timeout', type=int, default=30)
    parser.add_argument('--max_nodes', type=int, default=150000)
    parser.add_argument('--workers', type=int, default=os.cpu_count(),
                        help="Number of parallel processes")
    args = parser.parse_args()

    tasks = []

    # 1. First, build the task list and create directories
    print("Scanning directories...")
    for subdir, dirs, files in os.walk(args.input):
        smt_files = [f for f in files if f.endswith(('.smt2', '.smt'))]
        if not smt_files: continue

        rel_path = os.path.relpath(subdir, args.input)
        target_dir = os.path.join(args.output, rel_path)
        os.makedirs(target_dir, exist_ok=True)

        for filename in smt_files:
            file_path = os.path.join(subdir, filename)
            out_path = os.path.join(target_dir, filename.rsplit('.', 1)[0] + '.pt')

            if not os.path.exists(out_path):
                tasks.append((file_path, out_path, args.timeout, args.max_nodes))

    print(f"Starting parallel processing with {args.workers} workers for {len(tasks)} files...")

    stats = {"success": 0, "timeout": 0, "oversized": 0, "error": 0}
    start_time = time.time()

    # 2. Execute tasks in parallel
    with ProcessPoolExecutor(max_workers=args.workers) as executor:
        futures = {executor.submit(worker_task, t): t for t in tasks}

        failed_files = []
        for i, future in enumerate(as_completed(futures)):
            result = future.result()

            original_task_info = futures[future]
            file_path = original_task_info[0]
            if result == "success":
                stats["success"] += 1
            elif result == "timeout":
                stats["timeout"] += 1
            elif result == "oversized":
                stats["oversized"] += 1
            else:
                stats[result.split(':')[0]] += 1
                # Log the specific file and the error
                failed_files.append(f"{file_path} -> {result}")

            if (i + 1) % (len(tasks)//10) == 0:
                print(f"Progress: {i+1}/{len(tasks)} files complete...")

    with open("conversion_errors.log", "w") as f:
        for entry in failed_files:
            f.write(entry + "\n")

    print(f"\nSaved {len(failed_files)} errors to conversion_errors.log")

    duration = time.time() - start_time
    print(f"\n--- Parallel Processing Complete ---")
    print(f"Duration: {duration:.2f}s ({len(tasks)/duration:.2f} files/sec)")
    print(f"Stats: {stats}")

if __name__ == "__main__":
    main()
