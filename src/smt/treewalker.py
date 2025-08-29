from pysmt.walkers.tree import TreeWalker
import pysmt.operators as op

import sys
from pysmt.walkers.identitydag import IdentityDagWalker
from pysmt.smtlib.parser import SmtLibParser
from pysmt.operators import op_to_str, ALL_TYPES
from pysmt.fnode import FNode
import argparse
import networkx as nx
import numpy as np
from pysmt.exceptions import PysmtTypeError

import os


class GraphBuilder(TreeWalker):
    def __init__(self, env=None):
        super().__init__(env)
        self.counter_to_node_id = dict()
        self.node_id_to_counters = dict()
        self.var_to_value_node = dict()

        self.nodes = []
        self.edges = []
        self.edge_attrs = []

        self.parent = []

    def add_edge(self, paren_num, child_num, attr):
        self.edges.append((paren_num, child_num))
        self.edge_attrs.append(attr)

    def add_node(self, node_type:int):
        node_num = len(self.nodes)
        self.nodes.append(node_type)

        return node_num


    def walk_nary(self, formula:FNode):
        node_num = self.add_node(formula.node_type())

        if self.parent:
            self.add_edge(self.parent[-1], node_num, 0)

        self.parent.append(node_num)
        args = formula.args()
        for arg in args:
            yield arg
        
        self.parent.pop()

    def walk_constant(self, formula:FNode):
        node_num = self.add_node(formula.node_type())
        if self.parent:
            self.add_edge(self.parent[-1], node_num, 0)

        name = formula.__repr__()

        if name in self.var_to_value_node:
            value = self.var_to_value_node[name]
            self.add_edge(node_num, value, 1)
        else:
            value_num = self.add_node(len(ALL_TYPES))
            self.add_edge(node_num, value_num, 1)
            self.var_to_value_node[name] = value_num

    def walk_symbol(self, formula:FNode):
        node_num = self.add_node(formula.node_type())
        if self.parent:
            self.add_edge(self.parent[-1], node_num, 0)

        name = formula.symbol_name()

        if name in self.var_to_value_node:
            value = self.var_to_value_node[name]
            self.add_edge(node_num, value, 1)
        else:
            value_num = self.add_node(len(ALL_TYPES)+1)
            self.add_edge(node_num, value_num, 1)
            self.var_to_value_node[name] = value_num

    # N-ary ops 
    def walk_and(self, formula): return self.walk_nary(formula)
    def walk_and(self, formula): return self.walk_nary(formula)
    def walk_or(self, formula): return self.walk_nary(formula)
    def walk_plus(self, formula): return self.walk_nary(formula)
    def walk_times(self, formula): return self.walk_nary(formula)
    def walk_div(self, formula): return self.walk_nary(formula)
    def walk_pow(self, formula): return self.walk_nary(formula)
    def walk_iff(self, formula): return self.walk_nary(formula)
    def walk_implies(self, formula): return self.walk_nary(formula)
    def walk_minus(self, formula): return self.walk_nary(formula)
    def walk_equals(self, formula): return self.walk_nary(formula)
    def walk_le(self, formula): return self.walk_nary(formula)
    def walk_lt(self, formula): return self.walk_nary(formula)
    def walk_bv_xor(self, formula): return self.walk_nary(formula)
    def walk_bv_concat(self, formula): return self.walk_nary(formula)
    def walk_bv_udiv(self, formula): return self.walk_nary(formula)
    def walk_bv_urem(self, formula): return self.walk_nary(formula)
    def walk_bv_sdiv(self, formula): return self.walk_nary(formula)
    def walk_bv_srem(self, formula): return self.walk_nary(formula)
    def walk_bv_sle(self, formula): return self.walk_nary(formula)
    def walk_bv_slt(self, formula): return self.walk_nary(formula)
    def walk_bv_ule(self, formula): return self.walk_nary(formula)
    def walk_bv_ult(self, formula): return self.walk_nary(formula)
    def walk_bv_lshl(self, formula): return self.walk_nary(formula)
    def walk_bv_lshr(self, formula): return self.walk_nary(formula)
    def walk_bv_ashr(self, formula): return self.walk_nary(formula)
    def walk_bv_comp(self, formula): return self.walk_nary(formula)
    def walk_bv_and(self, formula): return self.walk_nary(formula)
    def walk_bv_or(self, formula): return self.walk_nary(formula)
    def walk_bv_not(self, formula): return self.walk_nary(formula)
    def walk_bv_add(self, formula): return self.walk_nary(formula)
    def walk_bv_mul(self, formula): return self.walk_nary(formula)
    def walk_bv_sub(self, formula): return self.walk_nary(formula)

    # Constants
    def walk_bv_constant(self, formula): return self.walk_constant(formula)
    def walk_int_constant(self, formula): return self.walk_constant(formula)
    def walk_real_constant(self, formula): return self.walk_constant(formula)
    def walk_int_constant(self, formula): return self.walk_constant(formula)
    def walk_bool_constant(self, formula): return self.walk_constant(formula)
    def walk_bv_constant(self, formula): return self.walk_constant(formula)
    def walk_algebraic_constant(self, formula): return self.walk_constant(formula)

    def walk_not(self, formula):
        return self.walk_nary(formula)

    def walk_function(self, formula):
        return self.walk_nary(formula)

    def walk_bv_extract(self, formula):
        return self.walk_nary(formula)

    def walk_bv_neg(self, formula):
        return self.walk_nary(formula)

    def walk_bv_ror(self, formula):
        return self.walk_nary(formula)

    def walk_bv_rol(self, formula):
        return self.walk_nary(formula)

    def walk_bv_zext(self, formula):
        return self.walk_nary(formula)

    def walk_bv_sext(self, formula):
        return self.walk_nary(formula)

    def walk_ite(self, formula):
        return self.walk_nary(formula)

    def walk_forall(self, formula):
        return self.walk_nary(formula)

    def walk_exists(self, formula):
        return self.walk_nary(formula)

    def walk_toreal(self, formula):
        return self.walk_nary(formula)

    def walk_str_constant(self, formula):
        return self.walk_nary(formula)

    def walk_str_length(self,formula):
        return self.walk_nary(formula)

    def walk_str_charat(self,formula, **kwargs):
        return self.walk_nary(formula)

    def walk_str_concat(self,formula, **kwargs):
        return self.walk_nary(formula)

    def walk_str_contains(self,formula, **kwargs):
        return self.walk_nary(formula)

    def walk_str_indexof(self,formula, **kwargs):
        return self.walk_nary(formula)

    def walk_str_replace(self,formula, **kwargs):
        return self.walk_nary(formula)

    def walk_str_substr(self,formula, **kwargs):
        return self.walk_nary(formula)

    def walk_str_prefixof(self,formula, **kwargs):
        return self.walk_nary(formula)

    def walk_str_suffixof(self,formula, **kwargs):
        return self.walk_nary(formula)

    def walk_str_to_int(self,formula, **kwargs):
        return self.walk_nary(formula)

    def walk_int_to_str(self,formula, **kwargs):
        return self.walk_nary(formula)

    def walk_array_select(self, formula):
        return self.walk_nary(formula)

    def walk_array_store(self, formula):
        return self.walk_nary(formula)

    def walk_array_value(self, formula):
        return self.walk_nary(formula)

    def walk_bv_tonatural(self, formula):
        return self.walk_nary(formula)

def main(parser):
    args = parser.parse_args()
    if os.path.exists(args.out):
        exit()
    else:
        print(args.out)

    file = args.file

    myParser = SmtLibParser()
    formula = None
    # try:
    formula = myParser.get_script(open(file)).get_last_formula()
    # except PysmtTypeError:
    #     badfile = open("badfile.txt", 'a')
    #     badfile.write(file+"\n")
    #     sys.exit(1)

    gb = GraphBuilder()

    gb.walk(formula)

    nodes = np.array(gb.nodes)
    edges = np.transpose(np.array(gb.edges))
    edge_attr = np.array(gb.edge_attrs)

    np.savez_compressed(args.out, nodes=nodes, edges=edges, edge_attr=edge_attr)

    if args.dot:
        graph = nx.DiGraph()

        for i, node in enumerate(gb.nodes):
            node_label = "Constant_Value_Node" if node == 66 else "Symbol_Value_Node" if node == 67 else op_to_str(node)
            graph.add_node(i, label=node_label)

        for edge, attr in zip(gb.edges, gb.edge_attrs):
            graph.add_edge(edge[0],edge[1], label=attr)

        nx.drawing.nx_pydot.write_dot(graph, "thing.dot")
        

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="SMT File -> Pytorch-Geometric Graph")
    parser.add_argument('--file', type=str, help="The file to be converted to a graph")
    parser.add_argument('--dot', action="store_true",help="The file to be converted to a graph")
    parser.add_argument('--out', type=str,help="The location to save the graph")
    main(parser=parser)