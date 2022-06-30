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
            g.add_edge(ptrToStr(lhs), ptrToStr(s));
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
            g.add_edge(ptrToStr(child), ptrToStr(lhs));
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
                        g.add_edge(ptrToStr(lhs), ptrToStr(fd));
                    }
                }
            }
        }
        else{
            g.add_edge(ptrToStr(child), ptrToStr(lhs));
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
            g.add_edge(ptrToStr(c->getDecl()), ptrToStr(c->getDecl()));
            
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
                g.add_edge(ptrToStr(d->getDecl()), ptrToStr(d->getDecl()));
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
            g.add_edge(ptrToStr(arg), ptrToStr(param));
        }
    }

    return true;
}


void DataFlowBuilder::reachingDefinitions(int iterations){
    
}