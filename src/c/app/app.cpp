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

#include <graph-builder/graph-builder.h>
#include <utils/utils.h>
#include <iostream>

using namespace clang;
using namespace llvm;
using namespace clang::tooling;


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
    
    return Tool.run(GraphBuilderActionFactory(AST.getValue(), ICFG.getValue(), Call.getValue(), Data.getValue(), chainLength.getValue(), outFile.getValue(), print.getValue()).get());
}