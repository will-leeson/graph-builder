#include <utils.h>
#include <algorithm>
#include <sstream>
#include <iostream>

void graph::add_node(std::string ptrStr, std::string nodeType){
    if(nodes.insert(ptrStr).second){
        nodePtrToNum[ptrStr] = nodes.size();
        nodePtrToType[ptrStr] = nodeType;
    }
}

void graph::add_edge(std::string fromPtrStr, std::string toPtrStr){
    outEdges.push_back(nodePtrToNum[fromPtrStr]);
    inEdges.push_back(nodePtrToNum[toPtrStr]);
}

std::set<std::string> graph::get_nodes() { return nodes; }
std::vector<int> graph::get_outEdges() { return outEdges; }
std::vector<int> graph::get_inEdges() { return inEdges; }