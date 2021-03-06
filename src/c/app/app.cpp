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
    explicit GraphBuilderConsumer(ASTContext *Context, SourceManager *Manager, bool buildAST, bool buildICFG, bool buildCall, bool buildData, int chainLength, std::string outFile, bool print) : VisitorAST(Context, 0), VisitorICFG(Context, 1), VisitorCallGraph(Context, 2), VisitorDataFlow(Context, Manager, 3), buildAST(buildAST), buildICFG(buildICFG), buildCall(buildCall), buildData(buildData), chainLength(chainLength), outFile(outFile), print(print) { }
    
    virtual void HandleTranslationUnit(clang::ASTContext &Context) override {
        auto unit =  Context.getTranslationUnitDecl();
        std::cout<<"Building AST"<<std::endl;
        VisitorAST.TraverseDecl(unit);
        graph g = VisitorAST.getGraph();

        //The AST is the basis of the graph. Even if we don't take its edges and nodes, we need some way to map nodes to their types and edges to nodes
        // g.setNodePtrToNum(VisitorAST.getGraph().getNodePtrToNum());
        if(buildICFG || buildData){
            std::cout<<"Building ICFG"<<std::endl;
            VisitorICFG.TraverseDecl(unit);
            if(buildICFG){
                g.mergeGraph(VisitorICFG.getGraph());
            }
            if(buildData){
                std::cout<<"Building Data"<<std::endl;
                VisitorDataFlow.TraverseDecl(unit);

                VisitorDataFlow.setGenKill(VisitorICFG.getGenKill());
                VisitorDataFlow.setReferences(VisitorICFG.getReferences());
                VisitorDataFlow.setICFG(VisitorICFG.getGraph());
                VisitorDataFlow.setMain(VisitorICFG.getMain());

                VisitorDataFlow.reachingDefinitions(chainLength);

                g.mergeGraph(VisitorDataFlow.getGraph());
            }
        }
        if(buildCall){
            std::cout<<"Building Call"<<std::endl;
            VisitorCallGraph.TraverseDecl(unit);
            g.mergeGraph(VisitorCallGraph.getGraph());
        }

        g.serializeGraph();
        if(outFile.size()>0){
            g.toFile(outFile);
        }
        if(print){
            g.printGraph();
        }
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
};

class GraphBuilderAction : public clang::ASTFrontendAction {
public:
    GraphBuilderAction(bool ast, bool icfg, bool call, bool data, int chainLength, std::string outFile, bool print) { 
        buildAST= ast;
        buildICFG = icfg;
        buildCall = call;
        buildData = data;
        mchainLength = chainLength;
        moutFile = outFile;
        mPrint = print;
    };

    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &Compiler, llvm::StringRef InFile) override{
        return std::unique_ptr<clang::ASTConsumer>(
            new GraphBuilderConsumer(&Compiler.getASTContext(), &Compiler.getSourceManager(), buildAST, buildICFG, buildCall, buildData, mchainLength, moutFile, mPrint));
        }

private:
    bool buildAST;
    bool buildICFG;
    bool buildCall;
    bool buildData;
    int mchainLength;
    std::string moutFile;
    bool mPrint;
};

std::unique_ptr<FrontendActionFactory> myNewFrontendActionFactory(bool ast, bool icfg, bool call, bool data, int chain, std::string outFile, bool print) {
  class SimpleFrontendActionFactory : public FrontendActionFactory {
    public:
        SimpleFrontendActionFactory(bool ast, bool icfg, bool call, bool data, int chain, std::string outFile, bool print) : mAST(ast), mICFG(icfg), mCall(call), mData(data), mChainLength(chain), mOutFile(outFile), mPrint(print) {}

        std::unique_ptr<FrontendAction> create() override {
            return std::make_unique<GraphBuilderAction>(mAST, mICFG, mCall, mData, mChainLength, mOutFile, mPrint);
        }

    private:
        bool mAST;
        bool mICFG;
        bool mCall;
        bool mData;
        int mChainLength;
        std::string mOutFile;
        bool mPrint;
  };
 
  return std::unique_ptr<FrontendActionFactory>(
      new SimpleFrontendActionFactory(ast, icfg, call, data, chain, outFile, print));
};


static cl::OptionCategory MyToolCategory("Specific Options");
static cl::opt<bool> AST("ast", cl::desc("Build AST"), cl::cat(MyToolCategory));
static cl::opt<bool> ICFG("icfg", cl::desc("Build ICFG (Intraprocedural Control Flow Graph)"), cl::cat(MyToolCategory));
static cl::opt<bool> Call("call", cl::desc("Build Call graph"), cl::cat(MyToolCategory));
static cl::opt<bool> Data("data", cl::desc("Build Data Dependency edges"), cl::cat(MyToolCategory));
static cl::opt<unsigned int> chainLength("chain-length", cl::desc("Data dependency chain length (default 0)"), cl::cat(MyToolCategory), cl::init(0));
static cl::opt<std::string> outFile("output-file", cl::desc("File to output graph to"), cl::cat(MyToolCategory));
static cl::opt<bool> print("print", cl::desc("Print to stdout"), cl::cat(MyToolCategory));

int main(int argc, const char **argv) {
    CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
    ClangTool Tool(OptionsParser.getCompilations(),
                    OptionsParser.getSourcePathList());
    
    return Tool.run(myNewFrontendActionFactory(AST.getValue(), ICFG.getValue(), Call.getValue(), Data.getValue(), chainLength.getValue(), outFile.getValue(), print.getValue()).get());
}