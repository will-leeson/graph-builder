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
#include <call/CallGraphBuilder.h>
#include <data/DataFlowBuilder.h>
#include <utils/utils.h>
#include <iostream>

using namespace clang;
using namespace llvm;
using namespace clang::tooling;

class GraphBuilderConsumer : public clang::ASTConsumer {
public:
    explicit GraphBuilderConsumer(ASTContext *Context, SourceManager *Manager, bool buildAST, bool buildICFG, bool buildCall, bool buildData, int chainLength, std::string outFile, bool print, graph *g) : VisitorAST(Context, 0), VisitorICFG(Context, 1), VisitorCallGraph(Context, 2), VisitorDataFlow(Context, Manager, 3), buildAST(buildAST), buildICFG(buildICFG), buildCall(buildCall), buildData(buildData), chainLength(chainLength), outFile(outFile), print(print), g(g) { }
    
    virtual void HandleTranslationUnit(clang::ASTContext &Context) override {
        auto unit =  Context.getTranslationUnitDecl();
        std::cout<<"Building AST"<<std::endl;
        VisitorAST.TraverseDecl(unit);
        graph localG = VisitorAST.getGraph();

        //The AST is the basis of the graph. Even if we don't take its edges and nodes, we need some way to map nodes to their types and edges to nodes
        // g.setNodePtrToNum(VisitorAST.getGraph().getNodePtrToNum());
        if(buildICFG || buildData){
            std::cout<<"Building ICFG"<<std::endl;
            VisitorICFG.TraverseDecl(unit);
            if(buildICFG){
                localG.mergeGraph(VisitorICFG.getGraph());
            }
            if(buildData){
                std::cout<<"Building Data"<<std::endl;
                VisitorDataFlow.TraverseDecl(unit);

                VisitorDataFlow.setGenKill(VisitorICFG.getGenKill());
                VisitorDataFlow.setReferences(VisitorICFG.getReferences());
                VisitorDataFlow.setICFG(VisitorICFG.getGraph());
                VisitorDataFlow.setMain(VisitorICFG.getMain());

                VisitorDataFlow.reachingDefinitions(chainLength);

                localG.mergeGraph(VisitorDataFlow.getGraph());
            }
        }
        if(buildCall){
            std::cout<<"Building Call"<<std::endl;
            VisitorCallGraph.TraverseDecl(unit);
            localG.mergeGraph(VisitorCallGraph.getGraph());
        }

        localG.serializeGraph();
        if(outFile.size()>0){
            localG.toFile(outFile);
        }
        if(print){
            localG.printGraph();
        }
        g = &localG;
    }
private:
    ASTBuilder VisitorAST;
    ICFGBuilder VisitorICFG;
    CallGraphBuilder VisitorCallGraph;
    DataFlowBuilder VisitorDataFlow;
    bool buildAST;
    bool buildICFG;
    bool buildCall;
    bool buildData;
    int chainLength;
    std::string outFile;
    bool print;
    graph *g;
};

class GraphBuilderAction : public clang::ASTFrontendAction {
public:
    GraphBuilderAction(bool ast, bool icfg, bool call, bool data, int chainLength, std::string outFile, bool print, graph *g) { 
        buildAST= ast;
        buildICFG = icfg;
        buildCall = call;
        buildData = data;
        mchainLength = chainLength;
        moutFile = outFile;
        mPrint = print;
        mG = g;
    };

    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &Compiler, llvm::StringRef InFile) override{
        return std::unique_ptr<clang::ASTConsumer>(
            new GraphBuilderConsumer(&Compiler.getASTContext(), &Compiler.getSourceManager(), buildAST, buildICFG, buildCall, buildData, mchainLength, moutFile, mPrint, mG));
        }

private:
    bool buildAST;
    bool buildICFG;
    bool buildCall;
    bool buildData;
    int mchainLength;
    std::string moutFile;
    bool mPrint;
    graph *mG;
};

std::unique_ptr<FrontendActionFactory> GraphBuilderActionFactory(bool ast, bool icfg, bool call, bool data, int chain, std::string outFile, bool print, graph *g) {
  class SimpleFrontendActionFactory : public FrontendActionFactory {
    public:
        SimpleFrontendActionFactory(bool ast, bool icfg, bool call, bool data, int chain, std::string outFile, bool print, graph* g) : mAST(ast), mICFG(icfg), mCall(call), mData(data), mChainLength(chain), mOutFile(outFile), mPrint(print), mG(g) {}

        std::unique_ptr<FrontendAction> create() override {
            return std::make_unique<GraphBuilderAction>(mAST, mICFG, mCall, mData, mChainLength, mOutFile, mPrint, mG);
        }

    private:
        bool mAST;
        bool mICFG;
        bool mCall;
        bool mData;
        graph *mG;
        int mChainLength;
        std::string mOutFile;
        bool mPrint;
  };
 
  return std::unique_ptr<FrontendActionFactory>(
      new SimpleFrontendActionFactory(ast, icfg, call, data, chain, outFile, print, g));
};