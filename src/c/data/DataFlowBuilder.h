#ifndef DATA_FLOW_BUILDER_H
#define DATA_FLOW_BUILDER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/Tooling/Tooling.h"
#include <set>
#include <utils/utils.h>

using namespace clang;
using namespace llvm;
using namespace clang::tooling;


class DataFlowBuilder : public RecursiveASTVisitor<DataFlowBuilder>{
public:
    explicit DataFlowBuilder(ASTContext *Context, SourceManager *Manager, int attr) : Context(Context), Manager(Manager), data(attr) {}

    DeclRefExpr * findDeclRefExpr(Stmt *s);
    void findReturnStmt(Decl *lhs, Stmt *s);

    void binaryOpHelper(Stmt *lhs, Stmt *child);
    void binaryOpHelper(Decl *lhs, Stmt *child);

    bool VisitBinaryOperator(BinaryOperator *b);
    bool VisitUnaryOperator(UnaryOperator *u);
    bool VisitVarDecl(VarDecl *v);
    bool VisitCallExpr(CallExpr *c);

    void reachingDefinitions(int iterations);
    void reachingPrep();

    void setReferences(std::map<std::string, std::string> ref);
    void setGenKill(std::map<std::string, std::pair<std::string, std::string>> gK);
    void setICFG(graph g);
    void setMain(std::string m);
    graph getGraph();


private:
    clang::ASTContext *Context;
    clang::SourceManager *Manager;
    graph data;
    graph icfg;
    std::string main;
    std::map<std::string, std::vector<std::string>> predecesors;
    std::map<std::string, std::vector<std::string>> succesors;
    std::map<std::string, std::string> references;
    std::map<std::string, std::pair<std::string, std::string>> genKill;
    std::map<std::pair<std::string, std::string>, int> edgeToStmtNum;
};

#endif //DATA_FLOW_BUILDER_H