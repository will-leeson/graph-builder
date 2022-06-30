#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/Tooling/Tooling.h"

class graph{
public:
    graph(){};

    void add_node(std::string ptrStr, std::string nodeType);

    void add_edge(std::string from, std::string to);

    std::set<std::string> get_nodes();
    std::vector<int> get_outEdges();
    std::vector<int> get_inEdges();
    std::map<std::string, int> nodePtrToNum;
    std::map<std::string, std::string> nodePtrToType;
private:
    std::set<std::string> nodes;
    std::vector<int> outEdges;
    std::vector<int> inEdges;
};

#endif //UTILS_H