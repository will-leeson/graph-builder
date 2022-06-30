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
    explicit DataFlowBuilder(ASTContext *Context, SourceManager *Manager, int attr) : Context(Context), Manager(Manager), g(attr) {}

    DeclRefExpr * findDeclRefExpr(Stmt *s);
    void findReturnStmt(Decl *lhs, Stmt *s);

    void binaryOpHelper(Stmt *lhs, Stmt *child);
    void binaryOpHelper(Decl *lhs, Stmt *child);

    bool VisitBinaryOperator(BinaryOperator *b);
    bool VisitUnaryOperator(UnaryOperator *u);
    bool VisitVarDecl(VarDecl *v);
    bool VisitCallExpr(CallExpr *c);

    void reachingDefinitions(int iterations);

private:
    clang::ASTContext *Context;
    clang::SourceManager *Manager;
    graph g;
};

#endif //DATA_FLOW_BUILDER_H