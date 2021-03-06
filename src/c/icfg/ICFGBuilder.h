#ifndef ICFG_BUILDER_H
#define ICFG_BUILDER_H

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

class ICFGBuilder : public RecursiveASTVisitor<ICFGBuilder>{
public:
    explicit ICFGBuilder(ASTContext *Context, int attr) : Context(Context), g(attr) {}
    
    DynTypedNode findSuccessor(const Stmt *node);
    void findReferencesHelper(const Stmt *orig, const DynTypedNode node);
    void findReferences(const Stmt *node);
    void printCFGPair(const Stmt* s1, const Stmt* s2);
    void printCFGPair(const Stmt* s, const Decl* d);
    void printCFGPair(const Decl* d, const Stmt* s);
    void printCFGPair(const Decl* d1, const Decl* d2);

    const Stmt* findPredecessor(DynTypedNode call, const CompoundStmt* compound);
    const Stmt* callExprHelper(DynTypedNode call);

    bool VisitCompoundStmt(CompoundStmt* cmpdStmt);
    bool VisitIfStmt(IfStmt *i);
    bool VisitSwitchStmt(SwitchStmt *s);
    bool VisitCaseStmt(CaseStmt *c);
    bool VisitDefaultStmt(DefaultStmt *d);
    bool VisitWhileStmt(WhileStmt *w);
    bool VisitDoStmt(DoStmt *d);
    bool VisitForStmt(ForStmt *f);
    bool VisitGotoStmt(GotoStmt *g);
    bool VisitBreakStmt(BreakStmt *b);
    bool VisitContinueStmt(ContinueStmt *c);
    bool VisitDecl(Decl *d);
    graph getGraph();
    std::map<std::string, std::string> getReferences();
    std::map<std::string, std::pair<std::string, std::string>> getGenKill();
    std::map<std::pair<std::string, std::string>, int> getEdgeToStmtNum();
    std::string getMain();

private:
    clang::ASTContext *Context;
    int stmtNumber = 0;
    graph g;
    std::string main;
    std::map<std::string, std::string> references;
    std::map<std::string, std::pair<std::string, std::string>> genKill;
    std::map<std::pair<std::string, std::string>, int> edgeToStmtNum;
};

#endif