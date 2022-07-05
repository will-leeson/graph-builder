#ifndef CALL_GRAPH_BUILDER_H
#define CALL_GRAPH_BUILDER_H

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

class CallGraphBuilder : public RecursiveASTVisitor<CallGraphBuilder>{
public:
    explicit CallGraphBuilder(ASTContext *Context, int attr) : Context(Context), g(attr) {}

    bool VisitCallExpr(CallExpr *call);

    graph getGraph();

private:
    clang::ASTContext *Context;
    graph g;
};

#endif
