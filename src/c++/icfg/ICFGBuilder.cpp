#include <ICFGBuilder.h>
#include <iostream>
#include <sstream>

#include <utils/utils.h>

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
                        references[stmtNumber] = std::pair<std::string, std::string>(ptrToStr(d),ptrToStr(d->getDecl()));
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
    std::string s1Str = ptrToStr(s1);
    std::string s2Str = ptrToStr(s2);

    g.add_edge(s1Str, s2Str);
    edgeToStmtNum[std::pair<std::string,std::string>(s1Str,s2Str)] = stmtNumber;
    if(isa<DeclStmt>(s1)){
        const DeclStmt *d = cast<DeclStmt>(s1);
        for(auto dPrime : d->decls()){
            if(isa<VarDecl>(dPrime)){
                VarDecl *v = cast<VarDecl>(dPrime);
                genKill[stmtNumber] = std::pair<std::string, std::string>(ptrToStr(d),ptrToStr(v));
            }
        }
    }
    else if(isa<BinaryOperator>(s1)){
        const BinaryOperator *b = cast<BinaryOperator>(s1);
        if(b->isAssignmentOp()){
            if(isa<DeclRefExpr>(b->getLHS())){
                DeclRefExpr *d = cast<DeclRefExpr>(b->getLHS());
                ValueDecl *dPrime = d->getDecl();
                genKill[stmtNumber] = std::pair<std::string, std::string>(ptrToStr(d),ptrToStr(dPrime));
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
                    genKill[stmtNumber] = std::pair<std::string, std::string>(ptrToStr(d),ptrToStr(dPrime));
                }
            }
        } 
    }
    findReferences(s1);
    stmtNumber++;
}

void ICFGBuilder::printCFGPair(const Stmt* s, const Decl* d){
    std::string sStr = ptrToStr(s);
    std::string dStr = ptrToStr(d);

    if(d){
        g.add_edge(sStr, dStr);
        edgeToStmtNum[std::pair<std::string,std::string>(sStr,dStr)] = stmtNumber;
    }
    else{
        return;
    }
    if(isa<DeclStmt>(s)){
        const DeclStmt *d = cast<DeclStmt>(s);
        for(auto dPrime : d->decls()){
            if(isa<VarDecl>(dPrime)){
                VarDecl *v = cast<VarDecl>(dPrime);
                genKill[stmtNumber] = std::pair<std::string, std::string>(ptrToStr(d),ptrToStr(v));
            }
        }
    }
    else if(isa<BinaryOperator>(s)){
        const BinaryOperator *b = cast<BinaryOperator>(s);
        if(b->isAssignmentOp()){
            if(isa<DeclRefExpr>(b->getLHS())){
                DeclRefExpr *d = cast<DeclRefExpr>(b->getLHS());
                ValueDecl *dPrime = d->getDecl();
                genKill[stmtNumber] = std::pair<std::string, std::string>(ptrToStr(d),ptrToStr(dPrime));
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
                    genKill[stmtNumber] = std::pair<std::string, std::string>(ptrToStr(d),ptrToStr(dPrime));
                }
            }
        }
    }
    findReferences(s);
    stmtNumber++;
}

void ICFGBuilder::printCFGPair(const Decl* d, const Stmt* s){
    std::string dStr = ptrToStr(d);
    std::string sStr = ptrToStr(s);

    g.add_edge(dStr, sStr);
    edgeToStmtNum[std::pair<std::string,std::string>(dStr,sStr)] = stmtNumber;
    stmtNumber++;
}

void ICFGBuilder::printCFGPair(const Decl* d1, const Decl* d2){
    std::string d1Str = ptrToStr(d1);
    std::string d2Str = ptrToStr(d2);

    g.add_edge(d1Str, d2Str);
    edgeToStmtNum[std::pair<std::string,std::string>(d1Str,d2Str)] = stmtNumber;
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
            std::string finalNodeStr = ptrToStr(finalNode);
            std::string succesorStr = ptrToStr(successor.get<FunctionDecl>());

            g.add_edge(finalNodeStr, succesorStr);
            edgeToStmtNum[std::pair<std::string,std::string>(finalNodeStr,succesorStr)] = stmtNumber;
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
            std::string finalNodeStr = ptrToStr(finalNode);
            std::string succesorStr = ptrToStr(successor.get<FunctionDecl>());

            g.add_edge(finalNodeStr, succesorStr);
            edgeToStmtNum[std::pair<std::string,std::string>(finalNodeStr,succesorStr)] = stmtNumber;
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
            std::string iStr = ptrToStr(i);
            std::string succesorStr = ptrToStr(successor.get<FunctionDecl>());

            g.add_edge(iStr, succesorStr);
            edgeToStmtNum[std::pair<std::string,std::string>(iStr,succesorStr)] = stmtNumber;
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
        std::string rStr = ptrToStr(r);
        std::string parentStr = ptrToStr(parent->get<FunctionDecl>());

        g.add_edge(rStr, parentStr);
        edgeToStmtNum[std::pair<std::string,std::string>(rStr,parentStr)] = stmtNumber;
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
        std::string finalNodeStr = ptrToStr(finalNode);
        std::string succesorStr = ptrToStr(successor.get<FunctionDecl>());

        g.add_edge(finalNodeStr, succesorStr);
        edgeToStmtNum[std::pair<std::string,std::string>(finalNodeStr,succesorStr)] = stmtNumber;
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
        std::string wStr = ptrToStr(w);
        std::string succesorStr = ptrToStr(successor.get<FunctionDecl>());

        g.add_edge(wStr, succesorStr);
        edgeToStmtNum[std::pair<std::string,std::string>(wStr,succesorStr)] = stmtNumber;
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
        std::string dStr = ptrToStr(d);
        std::string succesorStr = ptrToStr(successor.get<FunctionDecl>());

        g.add_edge(dStr, succesorStr);
        edgeToStmtNum[std::pair<std::string,std::string>(dStr,succesorStr)] = stmtNumber;
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
        std::string fStr = ptrToStr(f);
        std::string succesorStr = ptrToStr(successor.get<FunctionDecl>());

        g.add_edge(fStr, succesorStr);
        edgeToStmtNum[std::pair<std::string,std::string>(fStr,succesorStr)] = stmtNumber;
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
            std::string bStr = ptrToStr(b);
            std::string succesorStr = ptrToStr(successor.get<FunctionDecl>());

            g.add_edge(bStr, succesorStr);
            edgeToStmtNum[std::pair<std::string,std::string>(bStr,succesorStr)] = stmtNumber;
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
            std::string dStr = ptrToStr(d);
            std::string functionHeadStr = ptrToStr(functionHead);

            g.add_edge(dStr, functionHeadStr);
            edgeToStmtNum[std::pair<std::string,std::string>(dStr,functionHeadStr)] = stmtNumber;
            stmtNumber++;
        }
    }
    return true;
}

graph ICFGBuilder::getGraph(){ return g; }