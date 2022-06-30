#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include "clang/AST/ASTContext.h"

class graph{
public:
    graph(int attrVal): attrVal(attrVal) {};

    void add_node(std::string ptrStr, std::string nodeType);

    void add_edge(std::string from, std::string to);

    std::vector<std::string> get_nodes();
    std::set<std::string> get_nodeSet();
    std::vector<int> get_outEdges();
    std::vector<int> get_inEdges();
    std::map<std::string, int> nodePtrToNum;
    std::map<std::string, std::string> nodePtrToType;
    void set_nodes(std::vector<std::string> x);
    void serializeGraph();
private:
    std::set<std::string> nodeSet;
    std::vector<std::string> nodes;
    std::vector<int> outEdges;
    std::vector<int> inEdges;
    std::vector<int> edgeAttrs;
    int attrVal;
};

std::string ptrToStr(const clang::Decl* ptr);
std::string ptrToStr(const clang::Stmt* ptr);

#endif //UTILS_H