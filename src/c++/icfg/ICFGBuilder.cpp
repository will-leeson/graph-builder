#include <ICFGBuilder.h>
#include <iostream>

/*
    * Takes in a Stmt representing the node we want to find the successor of
    * While a successor hasn't been found, look through the generation of ancestors
    * until a ancestor has been found with a sibling stmt that comes after it. Return sibling.
    * If ancestor is a FunctionDecl, that node must be the last stmt in the function, so return
    * an end function node.
    */
DynTypedNode ICFGBuilder::findSuccessor(const Stmt *node){
    auto parent = Context->getParents(*node).begin();
    bool found = false;
    if(parent->get<Stmt>()){
        for(auto child : parent->get<Stmt>()->children()){
            if(found){
                return DynTypedNode::create(*child);
            }
            found = child == node;
        }
        return findSuccessor(parent->get<Stmt>());
    }
    else if(parent->get<Decl>()){
        return DynTypedNode::create(*(parent->get<Decl>()));
    }
    else{
        return DynTypedNode::create(*node);
    }     
}

void ICFGBuilder::findReferencesHelper(const Stmt *orig, const DynTypedNode node){
    if(node.get<Stmt>()){
        for(auto child : node.get<Stmt>()->children()){
            if(child!=NULL && !(isa<CompoundStmt>(child))){
                if(isa<DeclRefExpr>(child)){
                    const DeclRefExpr *d = cast<DeclRefExpr>(child);
                    if(isa<VarDecl>(d->getDecl())){   
                        std::cout<<"(Ref,"<<stmtNumber<<","<<orig<<",("<<d<<","<<d->getDecl()<<"))"<<std::endl;
                    }
                }
                else{
                    findReferencesHelper(orig, DynTypedNode::create(*child));
                }
            }
        }
    }
}

void ICFGBuilder::findReferences(const Stmt *node){
    if(node != NULL){
        if(isa<BinaryOperator>(node)){
            const BinaryOperator *b = cast<BinaryOperator>(node);
            if(b->isAssignmentOp()){
                DynTypedNode d = DynTypedNode::create(*(b->getRHS()));
                findReferencesHelper(node, d);
            }
            else{
                DynTypedNode d = DynTypedNode::create(*node);
                findReferencesHelper(node, d);
            }
        }
        else if (isa<CompoundStmt>(node)) {  }
        else{
            DynTypedNode d = DynTypedNode::create(*node);
            findReferencesHelper(node, d);
        }
    }
}

void ICFGBuilder::printCFGPair(const Stmt* s1, const Stmt* s2){
    std::cout<<"(CFG,"<<stmtNumber<<",("<<s1<<","<<s1->getStmtClassName()<<"),("<<s2<<","<<s2->getStmtClassName()<<"))"<<std::endl;
    if(isa<DeclStmt>(s1)){
        const DeclStmt *d = cast<DeclStmt>(s1);
        for(auto dPrime : d->decls()){
            if(isa<VarDecl>(dPrime)){
                VarDecl *v = cast<VarDecl>(dPrime);
                std::cout<<"(Gen/Kill,"<<stmtNumber<<",("<<d<<","<<v<<"))"<<std::endl;
            }
        }
    }
    else if(isa<BinaryOperator>(s1)){
        const BinaryOperator *b = cast<BinaryOperator>(s1);
        if(b->isAssignmentOp()){
            if(isa<DeclRefExpr>(b->getLHS())){
                DeclRefExpr *d = cast<DeclRefExpr>(b->getLHS());
                ValueDecl *dPrime = d->getDecl();
                std::cout<<"(Gen/Kill,"<<stmtNumber<<",("<<d<<","<<dPrime<<"))"<<std::endl;
            }
        }
    }
    else if(isa<UnaryOperator>(s1)){
        const UnaryOperator *u = cast<UnaryOperator>(s1);
        if(u->isIncrementDecrementOp()){
            for(auto child : u->children()){
                if(isa<DeclRefExpr>(child)){
                    const DeclRefExpr *d = cast<DeclRefExpr>(child);
                    const ValueDecl *dPrime = d->getDecl();
                    std::cout<<"(Gen/Kill,"<<stmtNumber<<",("<<d<<","<<dPrime<<")"<<std::endl;
                }
            }
        } 
    }
    findReferences(s1);
    stmtNumber++;
}

void ICFGBuilder::printCFGPair(const Stmt* s, const Decl* d){
    if(d){
        std::cout<<"(CFG,"<<stmtNumber<<",("<<s<<","<<s->getStmtClassName()<<"),("<<d<<","<<d->getDeclKindName()<<"))"<<std::endl;
    }
    else{
        return;
    }
    if(isa<DeclStmt>(s)){
        const DeclStmt *d = cast<DeclStmt>(s);
        for(auto dPrime : d->decls()){
            if(isa<VarDecl>(dPrime)){
                VarDecl *v = cast<VarDecl>(dPrime);
                std::cout<<"(Gen/Kill,"<<stmtNumber<<",("<<d<<","<<v<<"))"<<std::endl;
            }
        }
    }
    else if(isa<BinaryOperator>(s)){
        const BinaryOperator *b = cast<BinaryOperator>(s);
        if(b->isAssignmentOp()){
            if(isa<DeclRefExpr>(b->getLHS())){
                DeclRefExpr *d = cast<DeclRefExpr>(b->getLHS());
                ValueDecl *dPrime = d->getDecl();
                std::cout<<"(Gen/Kill,"<<stmtNumber<<",("<<d<<","<<dPrime<<"))"<<std::endl;
            }
        }
    }
    else if(isa<UnaryOperator>(s)){
        const UnaryOperator *u = cast<UnaryOperator>(s);
        if(u->isIncrementDecrementOp()){
            for(auto child : u->children()){
                if(isa<DeclRefExpr>(child)){
                    const DeclRefExpr *d = cast<DeclRefExpr>(child);
                    const ValueDecl *dPrime = d->getDecl();
                    std::cout<<"(Gen/Kill,"<<stmtNumber<<",("<<d<<","<<dPrime<<")"<<std::endl;
                }
            }
        }
    }
    findReferences(s);
    stmtNumber++;
}

void ICFGBuilder::printCFGPair(const Decl* d, const Stmt* s){
    std::cout<<"(CFG,"<<stmtNumber<<",("<<d<<","<<d->getDeclKindName()<<"),("<<s<<","<<s->getStmtClassName()<<"))"<<std::endl;
    stmtNumber++;
}

void ICFGBuilder::printCFGPair(const Decl* d1, const Decl* d2){
    std::cout<<"(CFG,"<<stmtNumber<<",("<<d1<<","<<d1->getDeclKindName()<<"),("<<d2<<","<<d2->getDeclKindName()<<"))"<<std::endl;
    stmtNumber++;
}

/*Links all items in a compound statement in order of execution*/
bool ICFGBuilder::VisitCompoundStmt(CompoundStmt* cmpdStmt){
    Stmt *prevChild = cmpdStmt;
    
    for(auto child : cmpdStmt->children()){
        bool isTakenCareOf = isa<IfStmt>(prevChild) || isa<BreakStmt>(prevChild) || WhileStmt::classof(prevChild) || 
                                isa<ForStmt>(prevChild) || isa<DoStmt>(prevChild) || isa<ContinueStmt>(prevChild) ||
                                SwitchStmt::classof(prevChild) || DefaultStmt::classof(prevChild);
        if(isTakenCareOf){
            prevChild = child;
        }
        else{
            printCFGPair(prevChild, child);
            prevChild = child;
        }   
    }
    
    return true;
}

const Stmt* ICFGBuilder::findPredecessor(DynTypedNode call, const CompoundStmt* compound){
    auto children = compound->children();
    const Stmt* favoriteChild = compound;
    bool found = false;
    for(auto child : children){
        found = child == call.get<Stmt>();
        if(found){
            return favoriteChild;
        }
        else{
            favoriteChild = child;
        }
    }
    return compound;
}

const Stmt* ICFGBuilder::callExprHelper(DynTypedNode call){
    auto parents = Context->getParents(call);

    for(auto parent : parents){
        if(parent.get<Stmt>()){
            auto s = parent.get<Stmt>();
            bool isValid = isa<IfStmt>(s) || isa<BreakStmt>(s) || WhileStmt::classof(s) || 
                                isa<ForStmt>(s) || isa<DoStmt>(s) || isa<ContinueStmt>(s) ||
                                SwitchStmt::classof(s) || DefaultStmt::classof(s) || isa<DeclStmt>(s) || isa<BinaryOperator>(s);
            if(isValid){
                return s;
            }
            else if(isa<CompoundStmt>(s)){
                return findPredecessor(call,cast<CompoundStmt>(s));
            }
            else{
                return callExprHelper(parent);
            }
        }
        else{
            return callExprHelper(parent);
        }
    }
    return NULL;
}

bool ICFGBuilder::VisitCallExpr(CallExpr *call){
    //Call graph edges: From callsite to function and back
    Decl* funDecl = call->getCalleeDecl();
    if(funDecl){
        printCFGPair(call, funDecl);
        printCFGPair(funDecl, call);
    }
    return true;
}

bool ICFGBuilder::VisitIfStmt(IfStmt *i){
    Stmt* thenStmt = i->getThen();

    DynTypedNode successor = findSuccessor(i);

    /* Link If header to then and else branch if they exist */
    if(thenStmt){
        printCFGPair(i, thenStmt);

        auto finalNode = thenStmt;
        if(isa<CompoundStmt>(thenStmt)){
            for(auto thenChild : thenStmt->children()){
                finalNode = thenChild;
            }
        }
        if(successor.get<Stmt>()){ 
            printCFGPair(finalNode, successor.get<Stmt>());
        }
        else if(successor.get<FunctionDecl>()){
            std::cout<<"(CFG,"<<stmtNumber<<",("<<finalNode<<","<<finalNode->getStmtClassName()<<"),("<<successor.get<FunctionDecl>()<<",FunctionExit))"<<std::endl;
            findReferences(finalNode);
            stmtNumber++;
        }
        else if(successor.get<Decl>()){
            printCFGPair(finalNode, successor.get<Decl>());
        }
    }

    Stmt* elseStmt = i->getElse();
    if(elseStmt){
        printCFGPair(i, elseStmt);

        auto finalNode = elseStmt;
        if(isa<CompoundStmt>(elseStmt)){
            for(auto elseChild : thenStmt->children()){
                finalNode = elseChild;
            }
        }
        if(successor.get<Stmt>()){ 
            printCFGPair(finalNode, successor.get<Stmt>());
        }
        else if(successor.get<FunctionDecl>()){
            std::cout<<"(CFG,"<<stmtNumber<<",("<<finalNode<<","<<finalNode->getStmtClassName()<<"),("<<successor.get<FunctionDecl>()<<",FunctionExit))"<<std::endl;
            findReferences(finalNode);
            stmtNumber++;
        }
        else if(successor.get<Decl>()){
            printCFGPair(finalNode, successor.get<Decl>());
        }
    }
    else{
        if(successor.get<Stmt>()){
            printCFGPair(i, successor.get<Stmt>());
        }
        else if(successor.get<FunctionDecl>()){
            std::cout<<"(CFG,"<<stmtNumber<<",("<<i<<","<<i->getStmtClassName()<<"),("<<successor.get<FunctionDecl>()<<",FunctionExit))"<<std::endl;
            findReferences(i);
            stmtNumber++;
        }
        else if(successor.get<Decl>()){
            printCFGPair(i, successor.get<Decl>());
        }
    }
    return true;
}

bool ICFGBuilder::VisitSwitchStmt(SwitchStmt *s){
    
    auto nextCase = s->getSwitchCaseList();
    while(nextCase){
        printCFGPair(s, nextCase);
        nextCase = nextCase->getNextSwitchCase();
    }

    return true;
}

bool ICFGBuilder::VisitCaseStmt(CaseStmt *c){
    auto body = c->getSubStmt();

    printCFGPair(c, body);
    return true;
}

bool ICFGBuilder::VisitReturnStmt(ReturnStmt *r){
    bool foundFunction = false;
    bool stopCondition = false;

    auto parent = Context->getParents(*r).begin();
    while(!stopCondition){
        foundFunction = parent->get<FunctionDecl>();
        stopCondition = (foundFunction || parent->get<TranslationUnitDecl>());
        if(stopCondition) break;

        if(parent->get<Stmt>()){
            parent = Context->getParents(*(parent->get<Stmt>())).begin();
        }
        else if(parent->get<Decl>()){
            parent = Context->getParents(*(parent->get<Decl>())).begin();
        }
    }

    if(parent->get<FunctionDecl>()){
        std::cout<<"(CFG,"<<stmtNumber<<",("<<r<<","<<r->getStmtClassName()<<"),("<<parent->get<FunctionDecl>()<<",FunctionExit))"<<std::endl;
        findReferences(r);
        stmtNumber++;
    }

    return true;
}

bool ICFGBuilder::VisitDefaultStmt(DefaultStmt *d){
    auto body = d->getSubStmt();

    printCFGPair(d, body);

    auto successor = findSuccessor(Context->getParents(*d).begin()->get<Stmt>());

    auto finalNode = body;
    if(isa<CompoundStmt>(body)){
        for(auto bodyChild : body->children()){
            finalNode = bodyChild;
        }
    }

    if(successor.get<Stmt>()){
        printCFGPair(finalNode, successor.get<Stmt>());
    }
    else if(successor.get<FunctionDecl>()){
        std::cout<<"(CFG,"<<stmtNumber<<",("<<finalNode<<","<<finalNode->getStmtClassName()<<"),("<<successor.get<FunctionDecl>()<<",FunctionExit))"<<std::endl;
        findReferences(finalNode);
        stmtNumber++;
    }
    else if(successor.get<Decl>()){
        printCFGPair(finalNode, successor.get<Decl>());
    }
    return true;
}

bool ICFGBuilder::VisitWhileStmt(WhileStmt *w){
    Stmt *body = w->getBody();
    
    //header to body
    printCFGPair(w, body);

    auto successor = findSuccessor(w);

    auto finalNode = body;
    if(isa<CompoundStmt>(body)){
        for(auto bodyChild : body->children()){
            finalNode = bodyChild;
        }
    }

    //body back to header
    printCFGPair(finalNode, w);

    //header to next node
    if(successor.get<Stmt>()){ 
        printCFGPair(w, successor.get<Stmt>());
    }
    else if(successor.get<FunctionDecl>()){
        std::cout<<"(CFG,"<<stmtNumber<<",("<<w<<","<<w->getStmtClassName()<<"),("<<successor.get<FunctionDecl>()<<",FunctionExit))"<<std::endl;
        findReferences(w);
        stmtNumber++;
    }
    else if(successor.get<Decl>()){
        printCFGPair(w, successor.get<Decl>());
    }
    return true;
}

bool ICFGBuilder::VisitDoStmt(DoStmt *d){
    Stmt *body = d->getBody();
    
    //header to body
    printCFGPair(d, body);

    auto successor = findSuccessor(d);

    auto finalNode = body;
    if(isa<CompoundStmt>(body)){
        for(auto bodyChild : body->children()){
            finalNode = bodyChild;
        }
    }

    //body back to header
    printCFGPair(finalNode, d);

    //header to next node
    if(successor.get<Stmt>()){ 
        printCFGPair(d, successor.get<Stmt>());
    }
    else if(successor.get<FunctionDecl>()){
        std::cout<<"(CFG,"<<stmtNumber<<",("<<d<<","<<d->getStmtClassName()<<"),("<<successor.get<FunctionDecl>()<<",FunctionExit))"<<std::endl;
        findReferences(d);
        stmtNumber++;
    }
    else if(successor.get<Decl>()){
        printCFGPair(d, successor.get<Decl>());
    }

    return true;
}

bool ICFGBuilder::VisitForStmt(ForStmt *f){
    Stmt* body = f->getBody();
    printCFGPair(f, body);

    auto successor = findSuccessor(f);

    auto finalNode = body;
    if(isa<CompoundStmt>(body)){
        for(auto bodyChild : body->children()){
            finalNode = bodyChild;
        }
    }

    //body back to header
    printCFGPair(finalNode, f);

    //header to next node
    if(successor.get<Stmt>()){ 
        printCFGPair(f, successor.get<Stmt>());
    }
    else if(successor.get<FunctionDecl>()){
        std::cout<<"(CFG,"<<stmtNumber<<",("<<f<<","<<f->getStmtClassName()<<"),("<<successor.get<FunctionDecl>()<<",FunctionExit))"<<std::endl;
        findReferences(f);
        stmtNumber++;
    }
    else if(successor.get<Decl>()){
        printCFGPair(f, successor.get<Decl>());
    }

    return true;
}

bool ICFGBuilder::VisitGotoStmt(GotoStmt *g){
    printCFGPair(g, g->getLabel()->getStmt());

    return true;
}

bool ICFGBuilder::VisitBreakStmt(BreakStmt *b){
    bool parentIsSwitch = false;
    bool parentIsWhile = false;
    bool parentIsCase = false;
    bool parentIsFor = false;
    bool parentIsDo = false;

    bool stopCondition = false;
    auto parent = Context->getParents(*b).begin();
    while(!stopCondition){
        parentIsSwitch = parent->get<Stmt>()->getStmtClass() == SwitchStmt::CreateEmpty(*Context, false, false)->getStmtClass();
        parentIsWhile = parent->get<WhileStmt>();
        parentIsCase = parent->get<CaseStmt>();
        parentIsFor = parent->get<ForStmt>();
        parentIsDo = parent->get<DoStmt>();

        stopCondition = (parentIsSwitch || parentIsCase || parentIsWhile || parentIsFor || parentIsDo || parent->get<TranslationUnitDecl>());
        
        if(parent->get<Stmt>()){
            parent = Context->getParents(*(parent->get<Stmt>())).begin();
        }
        else if(parent->get<Decl>()){
            parent = Context->getParents(*(parent->get<Decl>())).begin();
        }
    }
    if(parent->get<Stmt>()){
        auto successor = findSuccessor(parent->get<Stmt>());
        if(successor.get<Stmt>()){ 
            printCFGPair(b, successor.get<Stmt>());
        }
        else if(successor.get<FunctionDecl>()){
            std::cout<<"(CFG,"<<stmtNumber<<",("<<b<<","<<b->getStmtClassName()<<"),("<<successor.get<FunctionDecl>()<<",FunctionExit))"<<std::endl;
            stmtNumber++;
        }
        else if(successor.get<Decl>()){
            printCFGPair(b, successor.get<Decl>());
        }
    }
    return true;
}

bool ICFGBuilder::VisitContinueStmt(ContinueStmt *c){
    bool parentIsWhile = false;
    bool parentIsFor = false;
    bool parentIsDo = false;

    bool stopCondition = false;
    auto parent = Context->getParents(*c).begin();
    while(!stopCondition){
        parentIsWhile = parent->get<WhileStmt>();
        parentIsFor = parent->get<ForStmt>();
        parentIsDo = parent->get<DoStmt>();

        stopCondition = (parentIsWhile || parentIsFor || parentIsDo ||  parent->get<TranslationUnitDecl>());
        if(stopCondition) break;

        if(parent->get<Stmt>()){
            parent = Context->getParents(*(parent->get<Stmt>())).begin();
        }
        else if(parent->get<Decl>()){
            parent = Context->getParents(*(parent->get<Decl>())).begin();
        }
    }
    if(parent->get<Stmt>()){
        printCFGPair(c, parent->get<Stmt>());
    }

    return true;
}

bool ICFGBuilder::VisitDecl(Decl *d){
    if(isa<FunctionDecl>(d)){
        FunctionDecl* f = cast<FunctionDecl>(d);
        std::string stringRep = "Function";
        if(f->hasBody()){
            auto functionHead = f->getBody();
            if(f->getNameInfo().getAsString() == "main"){
                stringRep = "main";
            }
            std::cout<<"(CFG,"<<stmtNumber<<",("<<f<<","<<stringRep<<"),("<<functionHead<<","<<functionHead->getStmtClassName()<<"))"<<std::endl;
            stmtNumber++;
        }
    }
    return true;
}