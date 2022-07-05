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
    graph() {};

    void add_node(std::string ptrStr, std::string nodeType);

    void add_edge(std::string from, std::string to);
    void add_edge(std::string from, std::string to, int attr);

    std::vector<std::string> get_nodes();
    std::set<std::string> get_nodeSet();
    std::vector<std::string> get_outEdges();
    std::vector<std::string> get_inEdges();
    std::vector<int> get_edgeAttrs();
    std::map<std::string, int> nodePtrToNum;
    std::map<std::string, std::string> nodePtrToType;
    void set_nodes(std::vector<std::string> x);
    void serializeGraph();
    void printGraph();
    void mergeGraph(graph g);
private:
    std::set<std::string> nodeSet;
    std::vector<std::string> outEdges;
    std::vector<std::string> inEdges;

    std::vector<std::string> nodes;
    std::vector<int> outEdgesSerial;
    std::vector<int> inEdgesSerial;
    std::vector<int> edgeAttrs;
    int attrVal;
};

std::string ptrToStr(const clang::Decl* ptr);
std::string ptrToStr(const clang::Stmt* ptr);

#endif //UTILS_H