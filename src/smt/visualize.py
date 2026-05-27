import torch
import os
import sys
from pysmt import operators as ops

# --- Constants for ID Mapping ---
ID_VARIABLE = 66
ID_CONSTANT = 67

def get_op_name(op_id):
    """Maps the integer ID back to a pySMT token name or custom ID."""
    if op_id == ID_VARIABLE:
        return "VARIABLE_DEF"
    if op_id == ID_CONSTANT:
        return "CONSTANT_DEF"
    try:
        # returns strings like 'AND', 'BV_ADD', 'EQUALS'
        return ops.op_to_str(op_id)
    except:
        return f"UNKNOWN_{op_id}"

def export_to_dot(pt_file_path, out_dot_path):
    data = torch.load(pt_file_path, map_location='cpu', weights_only=False)
    
    with open(out_dot_path, 'w') as f:
        f.write("digraph SMT_AST {\n")
        f.write("  rankdir=TB;\n") # Top-to-Bottom
        f.write("  node [shape=box, style=filled, fontname=\"Courier\", fontsize=10];\n")
        
        colors = {
            "logic": "lightblue", "bitvector": "orange", "arithmetic": "lightgreen",
            "use": "tomato", "definition": "mediumpurple", "memory": "tan", "string": "pink"
        }

        identity_nodes = [] # To keep track of nodes we want at the bottom
        global_map = {}
        counter = 0

        # 1. Write all nodes
        for node_type in data.node_types:
            for i in range(data[node_type].x.shape[0]):
                node_id = f"n{counter}"
                global_map[(node_type, i)] = node_id
                op_id = data[node_type].x[i].item()
                
                name = get_op_name(op_id)
                color = colors.get(node_type, "grey")
                
                # If it's our Identity/Def nodes, we'll mark them for the bottom rank
                if node_type == "definition":
                    identity_nodes.append(node_id)
                
                f.write(f"  {node_id} [label=\"{name}\", fillcolor={color}];\n")
                counter += 1

        # 2. Force Identity nodes to the bottom level
        if identity_nodes:
            f.write("\n  { rank=sink; " + "; ".join(identity_nodes) + "; }\n")

        # 3. Write all edges
        for edge_type in data.edge_types:
            src_t, rel, dst_t = edge_type
            edge_index = data[edge_type].edge_index
            
            # Visual style to separate AST flow from Def-Use flow
            style = "dashed" if rel == "identity" else "solid"
            color = "gray40" if rel == "identity" else "black"
            
            for i in range(edge_index.shape[1]):
                u = global_map[(src_t, edge_index[0, i].item())]
                v = global_map[(dst_t, edge_index[1, i].item())]
                f.write(f"  {u} -> {v} [style={style}, color=\"{color}\"];\n")
        
        f.write("}\n")
    print(f"Refined Graphviz file exported to {out_dot_path}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        export_to_dot(sys.argv[1], "refined_output.dot")
    else:
        print("Usage: python script.py path/to/graph.pt")