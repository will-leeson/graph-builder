#include <utils.h>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>

void graph::add_node(std::string ptrStr, std::string nodeType){
    if(nodeSet.insert(ptrStr).second){
        nodePtrToNum[ptrStr] = nodeSet.size()-1;
        nodePtrToType[ptrStr] = nodeType;
    }
}

void graph::add_edge(std::string fromPtrStr, std::string toPtrStr){
    outEdges.push_back(fromPtrStr);
    inEdges.push_back(toPtrStr);
    edgeAttrs.push_back(attrVal);
}

void graph::add_edge(std::string fromPtrStr, std::string toPtrStr, int attr){
    outEdges.push_back(fromPtrStr);
    inEdges.push_back(toPtrStr);
    edgeAttrs.push_back(attr);
}

std::vector<std::string> graph::get_nodes() { return nodes; }
std::set<std::string> graph::get_nodeSet() { return nodeSet; }
std::vector<std::string> graph::get_outEdges() { return outEdges; }
std::vector<std::string> graph::get_inEdges() { return inEdges; }
std::vector<int> graph::get_edgeAttrs() { return edgeAttrs; }

void graph::set_nodes(std::vector<std::string> x){ nodes = x; }

std::map<std::string, int> graph::getNodePtrToNum(){ return nodePtrToNum; }

void graph::setNodePtrToNum(std::map<std::string, int> x){ nodePtrToNum = x; }

std::map<std::string, std::string> graph::getNodePtrToType(){ return nodePtrToType; }

void graph::setNodePtrToType(std::map<std::string, std::string> x){ nodePtrToType = x; }

void graph::serializeGraph(){
    std::vector <std::string> x(nodeSet.size());
    for(auto node : nodeSet){
        x[nodePtrToNum[node]] = nodePtrToType[node];
    }
    nodes = x;

    for(auto edge : inEdges){
        inEdgesSerial.push_back(nodePtrToNum[edge]);
    }
    for(auto edge : outEdges){
        outEdgesSerial.push_back(nodePtrToNum[edge]);
    }
}

void graph::printGraph(){
    std::cout<<"Nodes"<<std::endl;
    for(auto node : nodes){
        std::cout<<node<<std::endl;
    }
    std::cout<<"outEdge"<<std::endl;
    for(auto outEdge : outEdgesSerial){
        std::cout<<outEdge<<std::endl;
    }
    std::cout<<"inEdge"<<std::endl;
    for(auto inEdge : inEdgesSerial){
        std::cout<<inEdge<<std::endl;
    }
    std::cout<<"edgeAttr"<<std::endl;
    for(auto attr : edgeAttrs){
        std::cout<<attr<<std::endl;
    }
}

void graph::toFile(std::string fileName){
    std::ofstream outFile;
    outFile.open(fileName);
    outFile << "{\"nodes\": [";
    int i=0;
    for(; i<nodes.size()-1; i++){
        outFile << "\""<<nodes[i]<<"\"" << ", ";
    }
    outFile << "\""<< nodes[i] << "\"],";

    outFile <<" \"outEdges\": [";
    i=0;
    for(; i<outEdgesSerial.size()-1; i++){
        outFile << outEdgesSerial[i] << ", ";
    }
    outFile << outEdgesSerial[i] << "],";

    outFile <<" \"inEdges\": [";
    i=0;
    for(; i<inEdgesSerial.size()-1; i++){
        outFile << inEdgesSerial[i] << ", ";
    }
    outFile << inEdgesSerial[i] << "],";

    outFile <<" \"edgeAttr\": [";
    i=0;
    for(; i<edgeAttrs.size()-1; i++){
        outFile << edgeAttrs[i] << ", ";
    }
    outFile << edgeAttrs[i] << "]}";

}

void graph::mergeGraph(graph g){
    for(auto node : g.nodeSet){
        add_node(node, g.nodePtrToType[node]);
    }
    for(auto [outEdge, inEdge, attr] : llvm::zip(g.outEdges, g.inEdges, g.edgeAttrs)){
        add_edge(outEdge, inEdge, attr);
    }
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
