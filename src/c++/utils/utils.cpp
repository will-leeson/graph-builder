#include <utils.h>
#include <algorithm>
#include <sstream>
#include <iostream>

void graph::add_node(std::string ptrStr, std::string nodeType){
    if(nodeSet.insert(ptrStr).second){
        nodePtrToNum[ptrStr] = nodeSet.size()-1;
        nodePtrToType[ptrStr] = nodeType;
    }
}

void graph::add_edge(std::string fromPtrStr, std::string toPtrStr){
    outEdges.push_back(nodePtrToNum[fromPtrStr]);
    inEdges.push_back(nodePtrToNum[toPtrStr]);
    edgeAttrs.push_back(attrVal);
}

std::vector<std::string> graph::get_nodes() { return nodes; }
std::set<std::string> graph::get_nodeSet() { return nodeSet; }
std::vector<int> graph::get_outEdges() { return outEdges; }
std::vector<int> graph::get_inEdges() { return inEdges; }

void graph::set_nodes(std::vector<std::string> x){ nodes = x; }

void graph::serializeGraph(){
    std::vector <std::string> x(nodeSet.size());
    for(auto node : nodeSet){
        x[nodePtrToNum[node]] =  nodePtrToType[node];
    }

    nodes = x;
}

std::string ptrToStr(const clang::Stmt* ptr){
    std::ostringstream oss;
    oss << ptr;
    std::string ptrStr(oss.str());

    return ptrStr;
}

std::string ptrToStr(const clang::Decl* ptr){
    std::ostringstream oss;
    oss << ptr;
    std::string ptrStr(oss.str());

    return ptrStr;
}