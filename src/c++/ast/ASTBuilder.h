#ifndef AST_BUILDER_H
#define AST_BUILDER_H

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

class ASTBuilder : public RecursiveASTVisitor<ASTBuilder>{
public:
    explicit ASTBuilder(ASTContext *Context) : Context(Context) {}

    bool VisitStmt(Stmt *s);

    bool VisitDecl(Decl *d);

    void serializeGraph();

private:
    clang::ASTContext *Context;
    int placeholderVal = 0;
    graph ast;
};

#endif // AST_BUILDER_H