#include <DataFlowBuilder.h>
#include <iostream>

DeclRefExpr * DataFlowBuilder::findDeclRefExpr(Stmt *s){
    if(isa<DeclRefExpr>(s)){
        return cast<DeclRefExpr>(s);
    }
    else{
        for(auto child : s->children()){
            return findDeclRefExpr(child);
        }
        return nullptr;
    }
}

void DataFlowBuilder::findReturnStmt(Decl *lhs, Stmt *s){
    if(s){
        if(isa<ReturnStmt>(s)){
            data.add_edge(ptrToStr(lhs), ptrToStr(s));
            for(auto child : s->children()){
                binaryOpHelper(s, child);
            }
        }
        else{
            for(auto child : s->children()){
                findReturnStmt(lhs, child);
            }
        }
    }
}

void DataFlowBuilder::binaryOpHelper(Stmt *lhs, Stmt *child){
    if(child){
        if(isa<CallExpr>(child)){  }
        else{
            data.add_edge(ptrToStr(child), ptrToStr(lhs));
            if(!(child->children().empty())){
                for(auto childPrime : child->children()){
                    binaryOpHelper(lhs, childPrime);
                }
            }
        }
    }
}


void DataFlowBuilder::binaryOpHelper(Decl *lhs, Stmt *child){
    if(child){
        if(isa<CallExpr>(child)){
            
            auto call = cast<CallExpr>(child); 
            if(call->getCalleeDecl()){
                if(isa<FunctionDecl>(call->getCalleeDecl())){
                    
                    auto fd = cast<FunctionDecl>(call->getCalleeDecl()); 
                    if(fd->hasBody()){
                        findReturnStmt(lhs, fd->getBody());
                    }
                    else{
                        data.add_edge(ptrToStr(lhs), ptrToStr(fd));
                    }
                }
            }
        }
        else{
            data.add_edge(ptrToStr(child), ptrToStr(lhs));
            if(!(child->children().empty())){
                for(auto childPrime : child->children()){
                    binaryOpHelper(lhs, childPrime);
                }
            }
        }
    }
}

bool DataFlowBuilder::VisitBinaryOperator(BinaryOperator *b){
    if(b->isCompoundAssignmentOp()){
        if(isa<DeclRefExpr>(b->getLHS())){
            
            auto c = cast<DeclRefExpr>(b->getLHS());
            data.add_edge(ptrToStr(c->getDecl()), ptrToStr(c->getDecl()));
            
            binaryOpHelper(c->getDecl(), b->getRHS());
        }
        
    }
    else if(b->isAssignmentOp()){
        if(isa<DeclRefExpr>(b->getLHS())){
            
            auto c = cast<DeclRefExpr>(b->getLHS());
            binaryOpHelper(c->getDecl(), b->getRHS());
        }
        else{
            binaryOpHelper(b->getLHS(), b->getRHS());
        }
    }

    return true;
}

bool DataFlowBuilder::VisitUnaryOperator(UnaryOperator *u){
    if(u->isIncrementOp() || u->isDecrementOp()){
        for(auto child : u->children()){
            DeclRefExpr *d = findDeclRefExpr(child);
            if(d != NULL){
                data.add_edge(ptrToStr(d->getDecl()), ptrToStr(d->getDecl()));
            }
        }
    }
    return true;
}

bool DataFlowBuilder::VisitVarDecl(VarDecl *v){
    if(v->hasInit()){
        binaryOpHelper(v, v->getInit());
    }
    return true;
}

bool DataFlowBuilder::VisitCallExpr(CallExpr *c){
    auto d1 = c->getCalleeDecl();
    if(d1 && isa<FunctionDecl>(d1)){        
        auto d2 = cast<FunctionDecl>(d1);
        for(auto [param, arg] : zip(d2->parameters(),c->arguments())){
            data.add_edge(ptrToStr(arg), ptrToStr(param));
        }
    }

    return true;
}

void DataFlowBuilder::setReferences(std::map<std::string, std::string> ref){ references = ref; }
void DataFlowBuilder::setGenKill(std::map<std::string, std::pair<std::string, std::string>> gK){ genKill = gK; }
void DataFlowBuilder::setICFG(graph g){ icfg = g; }
void DataFlowBuilder::setMain(std::string m){ main = m; }


graph DataFlowBuilder::getGraph() { return data; }

void DataFlowBuilder::reachingPrep(){
    for(auto [outEdge, inEdge, attr] : zip(icfg.get_outEdges(), icfg.get_inEdges(), icfg.get_edgeAttrs())){
        if(predecesors.find(inEdge) != predecesors.end()){
            predecesors[inEdge].push_back(outEdge);
        }
        else{
            predecesors[inEdge] = {outEdge};
        }
        if(succesors.find(outEdge) != succesors.end()){
            succesors[outEdge].push_back(inEdge);
        }
        else{
            succesors[outEdge] = {inEdge};
        }
    }  
}

void printMapOfSets(std::map<std::string,std::set<std::string>> theMapSet){
    for(auto key : theMapSet){
        std::cout<<key.first<<" : ";
        for(auto item : key.second){
            std::cout<<item<<",";
        }
    }
}

void DataFlowBuilder::reachingDefinitions(int iterations){
    //Initialize OUT
    reachingPrep();
    std::map<std::string, std::map<std::string,std::set<std::string>>> OUT;
    std::set<std::string> nodes;
    bool changed = false;
    for(auto inEdge : icfg.get_inEdges()){
        if(genKill.find(inEdge) != genKill.end()){
            OUT[inEdge] = {{genKill.at(inEdge).second,{genKill.at(inEdge).first}}};
        }
        else{
            OUT[inEdge] = {};
        }
        nodes.insert(inEdge);
    }

    std::set<std::string> nextSet;
    for(int i=0; i< iterations; i++){
        for(auto node : nodes){
            std::map<std::string, std::set<std::string>> IN;

            if(predecesors.find(node) != predecesors.end()){
                for(auto p: predecesors.at(node)){
                    for(auto var : OUT[p]){
                        for(auto def : var.second){
                            if(IN.find(var.first) != IN.end()){
                                IN[var.first].insert(def);
                            }
                            else{
                                IN[var.first] = {def};
                            }
                        }
                    }
                }
            }

            auto oldOut = OUT[node];

            std::map<std::string, std::set<std::string>> newOut;
            if(genKill.find(node) != genKill.end()){
                IN[genKill[node].second] = {genKill[node].first};
            }
            OUT[node] = IN; 

            if(oldOut != OUT[node]){
                for(auto succesor : succesors[node]){
                    nextSet.insert(succesor);
                }
            }
        }   
        nodes = nextSet;
        nextSet = {};
        if(nodes.size()==0){
            break;
        }
    }

    std::map<std::string, std::map<std::string,std::set<std::string>>> finalOut;
    for(auto node: OUT){
        std::map<std::string, std::set<std::string>> aSet;
        for(auto pred: predecesors[node.first]){
            for(auto variable : OUT[pred]){
                if(aSet.find(variable.first) != aSet.end()){
                    for(auto def: variable.second){
                        aSet[variable.first].insert(def);
                    }
                }
                else{
                    aSet[variable.first] = variable.second;
                }
            }
        }
        finalOut[node.first] = aSet;
    }

    for(auto ref: references){
        if(finalOut.find(ref.first)!= finalOut.end()){
            for(auto item : finalOut[ref.first][ref.second]){
                data.add_edge(item, ref.second);
            }
        }
    }
}