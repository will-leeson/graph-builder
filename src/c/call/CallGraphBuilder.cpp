#include <CallGraphBuilder.h>

bool CallGraphBuilder::VisitCallExpr(CallExpr *call){
    //Call graph edges: From callsite to function and back
    Decl* funDecl = call->getCalleeDecl();
    if(funDecl){
        std::string callStr = ptrToStr(call);
        std::string funDeclStr = ptrToStr(funDecl);

        g.add_edge(callStr, funDeclStr);
        g.add_edge(funDeclStr, callStr);
    }
    return true;
}

graph CallGraphBuilder::getGraph() { return g; }
