#include <ASTBuilder.h>
#include <sstream>  
#include <iostream>
#include <vector>

bool ASTBuilder::VisitStmt(Stmt *s){
    const auto& parents = Context->getParents(*s);
    for(auto parent : parents){
        if(parent.get<Stmt>()){
            auto sPrime = parent.get<Stmt>();
            std::string parentStringRep;
            std::string stringRep;
            if(isa<BinaryOperator>(sPrime)){
                if(cast<BinaryOperator>(sPrime)->isCommaOp()){
                    parentStringRep = "CommaOperator";
                }
                else{
                    parentStringRep = cast<BinaryOperator>(sPrime)->getOpcodeStr().str();
                }
            }
            else if(isa<UnaryOperator>(sPrime)){
                parentStringRep = cast<UnaryOperator>(sPrime)->getOpcodeStr(cast<UnaryOperator>(sPrime)->getOpcode()).str();
            }
            else{
                std::string conversion1(sPrime->getStmtClassName());
                parentStringRep = conversion1;
            }
            if(isa<BinaryOperator>(s)){
                if(cast<BinaryOperator>(s)->isCommaOp()){
                    stringRep = "CommaOperator";
                }
                else{
                    stringRep =cast<BinaryOperator>(s)->getOpcodeStr().str();
                }
            }
            else if(isa<UnaryOperator>(s)){
                stringRep = cast<UnaryOperator>(s)->getOpcodeStr(cast<UnaryOperator>(s)->getOpcode()).str();
            }
            else{
                std::string conversion2(s->getStmtClassName());
                stringRep = conversion2;
            }
            
            std::string sPrimeStr = ptrToStr(sPrime);
            std::string sStr = ptrToStr(s);

            g.add_node(sPrimeStr, parentStringRep);
            g.add_node(sStr, stringRep);
            g.add_edge(sPrimeStr, sStr);
        }
        else if(parent.get<Decl>()){
            auto dPrime = parent.get<Decl>();

            std::string dPrimeStr = ptrToStr(dPrime);
            std::string sStr = ptrToStr(s);

            g.add_node(dPrimeStr, dPrime->getDeclKindName());
            g.add_node(sStr, s->getStmtClassName());
            g.add_edge(dPrimeStr, sStr);
        }
        else if(parent.get<TypeLoc>()){
            auto tPrime = parent.get<TypeLoc>();
            std::string stringRep;
            if(tPrime->getType().getTypePtr()->isArrayType()){
                if(isa<BinaryOperator>(s)){
                    if(cast<BinaryOperator>(s)->isCommaOp()){
                        stringRep = "CommaOperator";
                    }
                    else{
                        stringRep =cast<BinaryOperator>(s)->getOpcodeStr().str();
                    }
                }
                else if(isa<UnaryOperator>(s)){
                    stringRep = cast<UnaryOperator>(s)->getOpcodeStr(cast<UnaryOperator>(s)->getOpcode()).str();
                }
                else{
                    std::string conversion2(s->getStmtClassName());
                    stringRep = conversion2;
                }

                std::string sStr = ptrToStr(s);

                g.add_node("typePlaceholder"+std::to_string(placeholderVal), "array");
                g.add_node(sStr, stringRep);
                g.add_edge("typePlaceholder"+std::to_string(placeholderVal), sStr);
                placeholderVal++;
            }
        }
        else{
            errs()<<"I'm not sure what this is: " <<parent.getNodeKind().asStringRef().str()<<"\n";
        }
    }
    
    if(isa<DeclRefExpr>(s)){
        DeclRefExpr* d = cast<DeclRefExpr>(s);
        std::ostringstream oss;
        oss << s;
        std::string s1(oss.str());
        if(d->getDecl()->isImplicit() || g.getNodePtrToNum().find(s1) == g.getNodePtrToNum().end()){
            std::string sStr = ptrToStr(s);
            std::string dStr = ptrToStr(d->getDecl());             
            
            g.add_node(sStr, s->getStmtClassName());
            g.add_node(dStr, d->getDecl()->getDeclKindName());
            g.add_edge(sStr, dStr);
        }
    }

    return true;
}

bool ASTBuilder::VisitDecl(Decl *d){
    const auto& parents = Context->getParents(*d);
    std::string inputString = "VERIFIER_nondet";
    for(auto parent : parents){
        if(parent.get<Stmt>()){
            auto sPrime = parent.get<Stmt>();

            std::string sPrimeStr = ptrToStr(sPrime);
            std::string dStr = ptrToStr(d);

            g.add_node(sPrimeStr, sPrime->getStmtClassName());
            g.add_node(dStr, d->getDeclKindName());
            g.add_edge(sPrimeStr, dStr);
        }
        else if(parent.get<Decl>()){
            auto dPrime = parent.get<Decl>();
            std::string dPrimeStr = ptrToStr(dPrime);
            std::string dStr = ptrToStr(d);
            std::string paramStr;
            if(isa<FunctionDecl>(d)){
                FunctionDecl* f = cast<FunctionDecl>(d);
                if(f->getNameInfo().getAsString().find(inputString) != std::string::npos){
                    g.add_node(dPrimeStr, dPrime->getDeclKindName());
                    g.add_node(dStr, "input " + f->getReturnType().getAsString());
                    g.add_edge(dPrimeStr, dStr);
                }
                else if(f->isMain()){
                    g.add_node(dPrimeStr, dPrime->getDeclKindName());
                    g.add_node(dStr, "main");
                    g.add_edge(dPrimeStr, dStr);
                }
                else{
                    g.add_node(dPrimeStr, dPrime->getDeclKindName());
                    g.add_node(dStr, d->getDeclKindName());
                    g.add_edge(dPrimeStr, dStr);
                }
                for(auto param : f->parameters()){
                    paramStr = ptrToStr(param);
                    g.add_node(paramStr, param->getDeclKindName());
                    g.add_edge(dStr, paramStr);
                }
            }
            else{
                g.add_node(dPrimeStr, dPrime->getDeclKindName());
                g.add_node(dStr, d->getDeclKindName());
                g.add_edge(dPrimeStr, dStr);
            }
        }
    }

    if(isa<VarDecl>(d)){
        VarDecl* v = cast<VarDecl>(d);
        std::string dStr = ptrToStr(d); 

        if(v->getType().getTypePtr()->isBuiltinType()){
            g.add_node(dStr, d->getDeclKindName());
            g.add_node("typePlaceholder" + std::to_string(placeholderVal), v->getType().getDesugaredType(*Context).getAsString());
            g.add_edge(dStr, "typePlaceholder" + std::to_string(placeholderVal));
            placeholderVal++;
        }
        else if(v->getType().getTypePtr()->isStructureType()){
            g.add_node(dStr, d->getDeclKindName());
            g.add_node("typePlaceholder" + std::to_string(placeholderVal), "struct");
            g.add_edge(dStr, "typePlaceholder" + std::to_string(placeholderVal));
            if(v->hasInit()){
                std::string vStr = ptrToStr(v->getInit());
                g.add_node(vStr, v->getInit()->getStmtClassName());
                g.add_edge("typePlaceholder" + std::to_string(placeholderVal), vStr);
            }
            placeholderVal++;
        }
        else if(v->getType().getTypePtr()->isArrayType()){
            g.add_node(dStr, d->getDeclKindName());
            g.add_node("typePlaceholder" + std::to_string(placeholderVal), "array");
            g.add_edge(dStr, "typePlaceholder" + std::to_string(placeholderVal));
            if(v->hasInit()){
                std::string vStr = ptrToStr(v->getInit());
                g.add_node(vStr, v->getInit()->getStmtClassName());
                g.add_edge("typePlaceholder" + std::to_string(placeholderVal),vStr);
            }
            placeholderVal++;
        }
        else if(v->getType().getTypePtr()->isPointerType()){
            g.add_node(dStr, d->getDeclKindName());
            g.add_node("typePlaceholder" + std::to_string(placeholderVal), "pointer");
            g.add_edge(dStr, "typePlaceholder" + std::to_string(placeholderVal));
            placeholderVal++;
        }
        else{
            g.add_node(dStr, d->getDeclKindName());
            g.add_node("typePlaceholder" + std::to_string(placeholderVal), "otherType");
            g.add_edge(dStr, "typePlaceholder" + std::to_string(placeholderVal));
            placeholderVal++;
        }
    }

    return true;
}

graph ASTBuilder::getGraph() {return g; }