#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ParentMapContext.h"

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include <ast/ASTBuilder.h>
#include <icfg/ICFGBuilder.h>
#include <iostream>

using namespace clang;
using namespace llvm;
using namespace clang::tooling;

class GraphBuilderConsumer : public clang::ASTConsumer {
public:
    explicit GraphBuilderConsumer(ASTContext *Context, SourceManager *Manager, bool buildAST, bool buildICFG, bool buildCall, bool buildData, int chainLength) : VisitorAST(Context), VisitorICFG(Context) /*, VisitorDFG(Context, Manager)*/, buildAST(buildAST), buildICFG(buildICFG), buildCall(buildCall), buildData(buildData), chainLength(chainLength) {}
    virtual void HandleTranslationUnit(clang::ASTContext &Context) override {
        // VisitorDFG.TraverseDecl(Context.getTranslationUnitDecl());
        if(buildAST){
            VisitorAST.TraverseDecl(Context.getTranslationUnitDecl());
            std::cout<<"SERIALIZE"<<std::endl;
            VisitorAST.serializeGraph();
            std::cout<<"SERIALIZE"<<std::endl;
        }
        if(buildICFG){
            std::cout<<"To BUILD: ICFG"<<std::endl;
            VisitorICFG.TraverseDecl(Context.getTranslationUnitDecl());
        }
        if(buildCall){
            std::cout<<"To BUILD: Call"<<std::endl;
        }
        if(buildData){
            std::cout<<"To BUILD: Data"<<std::endl;
        }
    }

private:
    ASTBuilder VisitorAST;
    ICFGBuilder VisitorICFG;
    bool buildAST;
    bool buildICFG;
    bool buildCall;
    bool buildData;
    int chainLength;
};

class GraphBuilderAction : public clang::ASTFrontendAction {
public:
    GraphBuilderAction(bool ast, bool icfg, bool call, bool data, int chainLength) { 
        buildAST= ast;
        buildICFG = icfg;
        buildCall = call;
        buildData = data;
        chainLength = chainLength;
    };

    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &Compiler, llvm::StringRef InFile) override{
        return std::unique_ptr<clang::ASTConsumer>(
            new GraphBuilderConsumer(&Compiler.getASTContext(), &Compiler.getSourceManager(), buildAST, buildICFG, buildCall, buildData, chainLength));
        }
    
    void setBuildAST(bool build){ buildAST = build; }

private:
    bool buildAST;
    bool buildICFG;
    bool buildCall;
    bool buildData;
    int chainLength;
};

std::unique_ptr<FrontendActionFactory> myNewFrontendActionFactory(bool ast, bool icfg, bool call, bool data, int chain) {
  class SimpleFrontendActionFactory : public FrontendActionFactory {
    public:
        SimpleFrontendActionFactory(bool ast, bool icfg, bool call, bool data, int chain) : mAST(ast), mICFG(icfg), mCall(call), mData(data), mChainLength(chain) {}

        std::unique_ptr<FrontendAction> create() override {
            return std::make_unique<GraphBuilderAction>(mAST, mICFG, mCall, mData, mChainLength);
        }

    private:
        bool mAST;
        bool mICFG;
        bool mCall;
        bool mData;
        int mChainLength;
  };
 
  return std::unique_ptr<FrontendActionFactory>(
      new SimpleFrontendActionFactory(ast, icfg, call, data, chain));
};


static cl::OptionCategory MyToolCategory("Specific Options");
static cl::opt<bool> AST("ast", cl::desc("Build AST"), cl::cat(MyToolCategory));
static cl::opt<bool> ICFG("icfg", cl::desc("Build ICFG (Intraprocedural Control Flow Graph)"), cl::cat(MyToolCategory));
static cl::opt<bool> Call("call", cl::desc("Build Call graph"), cl::cat(MyToolCategory));
static cl::opt<bool> Data("data", cl::desc("Build Data Dependency edges"), cl::cat(MyToolCategory));
static cl::opt<unsigned int> chainLength("chain-length", cl::desc("Data dependency chain length (default 5)"), cl::cat(MyToolCategory), cl::init(5));

int main(int argc, const char **argv) {
    CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
    ClangTool Tool(OptionsParser.getCompilations(),
                    OptionsParser.getSourcePathList());
    
    return Tool.run(myNewFrontendActionFactory(AST.getValue(), ICFG.getValue(), Call.getValue(), Data.getValue(), chainLength.getValue()).get());
}