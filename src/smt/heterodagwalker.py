"""
DAG-aware counterpart to heterotreewalker.py.

heterotreewalker.py's SMT_HGT_Builder extends pysmt's TreeWalker, which
re-descends into a subformula every time it's referenced as a child --
identical shared subexpressions (extremely common in BMC-unrolled formulas)
get fully re-expanded at every occurrence. SMT_HGT_DagBuilder extends
pysmt's DagWalker instead: each distinct formula node (by pysmt's own
hash-consing identity) is visited and turned into a graph node exactly
once, and every reference to it becomes an edge to that same node --
producing the formula's true DAG rather than its tree expansion.

Output schema (node types, feature columns, edge relations) is identical
to heterotreewalker.py, so .pt files from either walker are interchangeable
inputs to the existing training pipeline / preprocessed-metadata.json.
Exception: operand_position=True (off by default) adds arg0/arg1/arg2/argN
edge relations for non-commutative operators -- see POSITIONAL_OPS -- so
those graphs need their own metadata.

One deliberate behavioral difference: walk_constant in the tree walker
creates a fresh "logic" wrapper node per occurrence of a constant (only
the underlying "definition" node was shared, via const_map). Here, since
walk_constant now runs exactly once per distinct constant formula, both
the "logic" wrapper and the "definition" node are shared -- one pair per
distinct constant value, referenced by every occurrence. var_map/const_map
are kept as explicit, name/repr-keyed dedup (matching the tree walker's
exact keys) rather than relying solely on DagWalker's formula-identity
memoization, to keep this a minimal, easily-auditable behavioral diff.

DagWalker's iter_walk is stack-based (not recursive), so the deep
recursion this file's formulas previously needed (see the
sys.setrecursionlimit in heterotreewalker.py) should no longer be a
concern -- kept below anyway as a harmless safety net.
"""

import os, sys
import time
import signal
import torch
import argparse, traceback
from collections import defaultdict
from concurrent.futures import ProcessPoolExecutor, as_completed
from pysmt import operators as ops
from torch_geometric.data import HeteroData
from pysmt.walkers.dag import DagWalker
import re
import io
import traceback
from pysmt.shortcuts import reset_env
from pysmt.smtlib.parser import SmtLibParser
from pysmt.smtlib import commands as smtcmd

sys.setrecursionlimit(20000)

# --- Timeout Configuration ---
class TimeoutException(Exception): pass

def timeout_handler(signum, frame):
    raise TimeoutException()

ID_VARIABLE = 66
ID_CONSTANT = 67

# Operators whose meaning depends on argument order. With operand_position
# on, their parent->child edges are named arg0/arg1/... (slot in
# formula.args() order) instead of "child", so HeteroConv gets a separate
# conv per slot and bvsub(a, b) no longer looks like bvsub(b, a).
# Deliberately excluded: commutative ops (AND/OR/EQUALS/IFF/PLUS/TIMES/
# BV_ADD/BV_MUL/BV_AND/BV_OR/BV_XOR/BV_COMP), unary ops (one slot, nothing
# to distinguish), and ARRAY_VALUE (default value + index/value pairs, so a
# flat slot index would be misleading). AND in particular must stay "child":
# the synthetic root AND's child order IS the assertion order, and labelling
# it would leak the variant scramble into the graph (the same signal
# assertion-PE adds, which was settled against for BMC).
POSITIONAL_OPS = frozenset({
    ops.BV_SUB, ops.BV_ULT, ops.BV_ULE, ops.BV_SLT, ops.BV_SLE,
    ops.BV_LSHL, ops.BV_LSHR, ops.BV_ASHR,
    ops.BV_UDIV, ops.BV_UREM, ops.BV_SDIV, ops.BV_SREM, ops.BV_CONCAT,
    ops.ITE, ops.IMPLIES, ops.LE, ops.LT, ops.MINUS, ops.DIV, ops.POW,
    ops.ARRAY_SELECT, ops.ARRAY_STORE, ops.FUNCTION,
    ops.STR_CONCAT, ops.STR_CONTAINS, ops.STR_INDEXOF, ops.STR_REPLACE,
    ops.STR_SUBSTR, ops.STR_PREFIXOF, ops.STR_SUFFIXOF, ops.STR_CHARAT,
})

# Slots >= this share one "argN" relation. Only variadic ops (uninterpreted
# FUNCTION applications, str.++) reach it; without a cap every new arity
# would mint new edge types that are rare in training and possibly unseen
# at test time.
MAX_ARG_SLOT = 3


def _arg_relation(i):
    return f"arg{i}" if i < MAX_ARG_SLOT else "argN"

def get_bv_width(formula):
    # Own type first (covers bv ops, bv constants, bv symbols, ite/select/
    # function returning BV). Falls back to the first BV-typed argument,
    # which is what relations like bv_sle/equals need since their own
    # result type is Bool. Non-BV formulas (and leaves with no args) get 0.
    ty = formula.get_type()
    if ty.is_bv_type():
        return ty.width
    for arg in formula.args():
        if arg.get_type().is_bv_type():
            return arg.get_type().width
    return 0

class SMT_HGT_DagBuilder(DagWalker):
    def __init__(self, env=None, max_nodes=100000, operand_position=False):
        super().__init__(env)
        self.max_nodes = max_nodes
        self.operand_position = operand_position
        self.node_count = 0

        self.type_map = {}
        self._setup_type_map()

        self.node_features = {
            "logic": [], "arithmetic": [], "bitvector": [],
            "memory": [], "string": [], "definition": []
        }

        self.edge_indices = {}
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
        # Now bounds the true DAG size rather than the tree-expanded size,
        # since add_node is only called once per distinct formula (each
        # walk_X below runs exactly once per node thanks to DagWalker's
        # memoization).
        self.node_count += 1
        if self.node_count > self.max_nodes:
            raise MemoryError(f"Graph exceeded max node limit of {self.max_nodes}")

        idx = len(self.node_features[node_type])
        self.node_features[node_type].append(feature)
        return idx

    def add_hetero_edge(self, src_type, dst_type, src_idx, dst_idx, rel_name="child"):
        rel = (src_type, rel_name, dst_type)
        if rel not in self.edge_indices:
            self.edge_indices[rel] = [[], []]
        self.edge_indices[rel][0].append(src_idx)
        self.edge_indices[rel][1].append(dst_idx)

    # Each walk_X below is called exactly once per distinct formula node
    # (DagWalker memoizes by formula identity). `args` holds the already-
    # computed (node_type, node_idx) results for each child, in the same
    # order as formula.args() -- including one entry per argument slot
    # even if two slots reference the same child, so multiplicity of
    # references is preserved. Each method is responsible for creating
    # its own node and wiring edges to its children; it returns its own
    # (node_type, node_idx) for whichever parent(s) reference it.

    def walk_nary(self, formula, args, **kwargs):
        this_type = self.get_node_type(formula)
        this_idx = self.add_node(this_type, [formula.node_type(), get_bv_width(formula)])

        positional = self.operand_position and formula.node_type() in POSITIONAL_OPS
        for i, (child_type, child_idx) in enumerate(args):
            rel_name = _arg_relation(i) if positional else "child"
            self.add_hetero_edge(this_type, child_type, this_idx, child_idx, rel_name)

        return (this_type, this_idx)

    def walk_symbol(self, formula, args, **kwargs):
        name = formula.symbol_name()
        if name not in self.var_map:
            try:
                var_num = int(name[1:])  # extract index from normalized x### form
            except (ValueError, IndexError):
                var_num = -1
            sym_type = formula.symbol_type()
            width = sym_type.width if sym_type.is_bv_type() else 0
            self.var_map[name] = self.add_node("definition", [ID_VARIABLE, var_num, width])

        return ("definition", self.var_map[name])

    def walk_constant(self, formula, args, **kwargs):
        width = get_bv_width(formula)
        this_idx = self.add_node("logic", [formula.node_type(), width])

        c_repr = str(formula)
        if c_repr not in self.const_map:
            self.const_map[c_repr] = self.add_node("definition", [ID_CONSTANT, 0, width])

        const_idx = self.const_map[c_repr]
        rel = ("definition", "identity", "logic")
        if rel not in self.edge_indices: self.edge_indices[rel] = [[], []]
        self.edge_indices[rel][0].append(const_idx)
        self.edge_indices[rel][1].append(this_idx)

        return ("logic", this_idx)

    # N-ary maps
    def walk_and(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_or(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_plus(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_times(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_div(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_pow(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_iff(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_implies(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_minus(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_equals(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_le(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_lt(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_xor(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_concat(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_udiv(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_urem(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_sdiv(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_srem(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_sle(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_slt(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_ule(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_ult(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_lshl(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_lshr(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_ashr(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_comp(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_and(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_or(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_not(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_add(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_mul(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_sub(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_constant(self, f, args, **k): return self.walk_constant(f, args, **k)
    def walk_int_constant(self, f, args, **k): return self.walk_constant(f, args, **k)
    def walk_real_constant(self, f, args, **k): return self.walk_constant(f, args, **k)
    def walk_bool_constant(self, f, args, **k): return self.walk_constant(f, args, **k)
    def walk_algebraic_constant(self, f, args, **k): return self.walk_constant(f, args, **k)
    def walk_not(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_function(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_extract(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_neg(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_ror(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_rol(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_zext(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_sext(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_ite(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_forall(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_exists(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_toreal(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_str_constant(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_str_length(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_str_charat(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_str_concat(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_str_contains(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_str_indexof(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_str_replace(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_str_substr(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_str_prefixof(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_str_suffixof(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_str_to_int(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_int_to_str(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_array_select(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_array_store(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_array_value(self, f, args, **k): return self.walk_nary(f, args, **k)
    def walk_bv_tonatural(self, f, args, **k): return self.walk_nary(f, args, **k)

    def walk_with_assertion_tracking(self, assertions):
        """
        Walk each top-level assertion FNode separately (call this with the
        list from sanitize_and_parse_assertions() instead of a single
        walk(get_last_formula())), recording which assertion(s) reference
        each resulting graph node.

        Produces the IDENTICAL graph to walking the merged And(*assertions)
        in one call: DagWalker.memoization (see pysmt's dag.py) is a
        persistent instance attribute created once in __init__ and never
        cleared here (invalidate_memoization defaults False and is never
        passed), so a subexpression built while walking assertion i is
        found in the cache and reused, not rebuilt, when assertion j
        references it -- splitting one big walk() into K per-assertion
        walk() calls changes nothing about what nodes/edges get created,
        only lets us observe which call(s) touched each one.

        Populates self.node_assertions: {(node_type, node_idx) -> set of
        assertion indices that reference that node}. A node touched by
        exactly one assertion is "private" to it (has a well-defined
        position); touched by 2+ is "shared" (e.g. a hub variable
        referenced by nearly every assertion in a query never has a single
        position -- see to_pyg's assertion_pe column).
        """
        if not hasattr(self, "node_assertions"):
            self.node_assertions = defaultdict(set)

        assertion_roots = []
        for i, a in enumerate(assertions):
            assertion_roots.append(self.walk(a))

            # Iterative (not recursive) traversal of this assertion's own
            # subformula tree -- a local visited-set guards against
            # revisiting a subexpression shared WITHIN this one assertion
            # (also a DAG, not just across assertions) more than once.
            seen = set()
            stack = [a]
            while stack:
                f = stack.pop()
                if f in seen:
                    continue
                seen.add(f)
                key = self.memoization.get(f)
                if key is not None:
                    self.node_assertions[key].add(i)
                stack.extend(f.args())

        # sanitize_and_parse's single walk(get_last_formula()) implicitly
        # creates a top-level AND(a0, ..., aK) node (get_last_formula()
        # And-merges every assertion) with a "child" edge to each
        # assertion's root -- walking assertions separately here never
        # builds that merged formula, so without this the resulting graph
        # would be missing exactly that one node relative to the existing
        # pipeline. Recreated directly (ops.AND, width 0 -- boolean, not a
        # bitvector) rather than via walk_nary since there's no single
        # FNode spanning all K assertions in this code path. Touched by
        # every assertion by construction, so it's correctly "shared"
        # (assertion_idx -1, no PE contribution) under the same
        # _assertion_idx_for logic as any other multi-assertion node --
        # no special-casing needed.
        root_idx = self.add_node("logic", [ops.AND, 0])
        for i, (child_type, child_idx) in enumerate(assertion_roots):
            self.add_hetero_edge("logic", child_type, root_idx, child_idx)
            self.node_assertions[("logic", root_idx)].add(i)

    @staticmethod
    def _assertion_idx_for(key, node_assertions):
        s = node_assertions.get(key)
        if s is not None and len(s) == 1:
            return next(iter(s))
        return -1  # shared across >=2 assertions, or untracked

    def to_pyg(self, assertion_pe=False):
        """
        assertion_pe: if True, appends one extra feature column to every
        node type -- the 0-indexed (file order) assertion it's private to,
        or -1 if shared across >=2 assertions (or walk_with_assertion_
        tracking was never called, in which case every node gets -1: a
        safe, uninformative default, never a wrong position). Consumed by
        SMTNodeEmbedding's assertion-position sinusoidal PE term.
        """
        data = HeteroData()
        node_assertions = getattr(self, "node_assertions", {})
        for node_type, features in self.node_features.items():
            if not features:
                continue
            if assertion_pe:
                features = [
                    list(feat) + [self._assertion_idx_for((node_type, idx), node_assertions)]
                    for idx, feat in enumerate(features)
                ]
            t = torch.tensor(features, dtype=torch.long)
            data[node_type].x = t if t.dim() > 1 else t.view(-1, 1)
        for (src, rel, dst), indices in self.edge_indices.items():
            data[src, rel, dst].edge_index = torch.tensor(indices, dtype=torch.long)
        return data

def _sanitize_content(file_path):
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
    # These are not valid SMT-LIB. '#b'/'#x' literals are safe -- they
    # contain non-'#' characters so won't match.
    joined = [l for l in joined if not all(c == '#' for c in l.strip()) or not l.strip()]
    content = ''.join(joined)

    # Z3_PRE_BLAST_DUMP prints Z3's internal goal representation directly,
    # which can leak Z3-internal "total"/interpreted division symbols
    # (bvudiv_i, bvurem_i, bvsdiv_i, bvsrem_i, bvsmod_i) that are never
    # declared in the file and aren't part of the SMT-LIB2 spec -- only
    # bvudiv/bvurem/bvsdiv/bvsrem/bvsmod are. Z3's internal total variants
    # agree with the spec's own (fully-specified) div-by-zero behavior, so
    # aliasing is safe, not just a parse-error workaround (verified against
    # bv_rewriter.cpp: under hi_div0, Z3 emits OP_BS{DIV,REM,MOD}_I bare,
    # the same code path as the unsigned OP_BU{DIV,REM}_I case below).
    content = re.sub(r'\bbvudiv_i\b', 'bvudiv', content)
    content = re.sub(r'\bbvurem_i\b', 'bvurem', content)
    content = re.sub(r'\bbvsdiv_i\b', 'bvsdiv', content)
    content = re.sub(r'\bbvsrem_i\b', 'bvsrem', content)
    content = re.sub(r'\bbvsmod_i\b', 'bvsmod', content)
    return content


def _build_script(content):
    parser = SmtLibParser()
    _patch_nary_bv_bitwise_ops(parser)
    return parser.get_script(io.StringIO(content))


def sanitize_and_parse(file_path):
    script = _build_script(_sanitize_content(file_path))
    return script.get_last_formula()


def sanitize_and_parse_assertions(file_path):
    """
    Like sanitize_and_parse, but returns the list of top-level assertion
    FNodes in file order instead of one And-merged formula --
    get_last_formula() discards assertion boundaries, which
    walk_with_assertion_tracking needs back. Same sanitization/parser
    setup as sanitize_and_parse, so produces identical parsed FNodes for
    any subexpression the two would share.
    """
    script = _build_script(_sanitize_content(file_path))
    return [cmd.args[0] for cmd in script.commands if cmd.name == smtcmd.ASSERT]


def _patch_nary_bv_bitwise_ops(parser):
    """
    pysmt's SmtLibParser dispatches 'bvand'/'bvor'/'bvxor' straight to
    FormulaManager.BVAnd/BVOr/BVXor, which are strictly binary (see
    _operator_adapter in pysmt/smtlib/parser/parser.py -- it pushes the
    method reference as-is, with no arity handling; the actual call site,
    parser.py's get_expression, does `fun(*lst)` with however many operands
    the s-expression had). Z3_PRE_BLAST_DUMP can print an n-ary bvxor/bvand/
    bvor (its rewriter flattens chains of these into one node), which then
    blows up with e.g. "BVXor() takes 3 positional arguments but 9 were
    given". All three are associative+commutative, so left-folding the
    n-ary form into nested binary calls is exactly semantics-preserving,
    not an approximation.
    """
    mgr = parser.env.formula_manager
    for name, binop in (("bvand", mgr.BVAnd), ("bvor", mgr.BVOr), ("bvxor", mgr.BVXor)):
        def nary(stack, tokens, key, _binop=binop):
            args = stack[-1]
            def fold(*a, _binop=_binop):
                result = a[0]
                for x in a[1:]:
                    result = _binop(result, x)
                return result
            args.append(fold)
        parser.interpreted[name] = nary


def worker_task(file_info):
    # 6th element optional so older 5-tuple callers keep working unchanged.
    file_path, out_path, timeout, max_nodes, assertion_pe, *rest = file_info
    operand_position = rest[0] if rest else False

    def local_timeout_handler(signum, frame): raise TimeoutException()
    signal.signal(signal.SIGALRM, local_timeout_handler)
    signal.alarm(timeout)

    try:
        reset_env()

        gb = SMT_HGT_DagBuilder(max_nodes=max_nodes, operand_position=operand_position)
        if assertion_pe:
            assertions = sanitize_and_parse_assertions(file_path)
            gb.walk_with_assertion_tracking(assertions)
        else:
            formula = sanitize_and_parse(file_path)
            gb.walk(formula)
        data = gb.to_pyg(assertion_pe=assertion_pe)
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
    parser.add_argument('--assertion_pe', action='store_true', default=False,
                        help="Walk assertions separately and append a trailing "
                             "per-node assertion-index feature column (-1 if "
                             "shared across >=2 assertions), for the assertion-"
                             "position sinusoidal PE term in SMTNodeEmbedding. "
                             "Default off -- existing production graphs/pipelines "
                             "are unaffected.")
    parser.add_argument('--operand_position', action='store_true', default=False,
                        help="Name parent->child edges of non-commutative "
                             "operators arg0/arg1/arg2/argN by operand slot "
                             "instead of 'child' (see POSITIONAL_OPS). Default "
                             "off -- existing graphs are unaffected.")
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
                tasks.append((file_path, out_path, args.timeout, args.max_nodes,
                              args.assertion_pe, args.operand_position))

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

            if len(tasks) >= 10 and (i + 1) % (len(tasks)//10) == 0:
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
