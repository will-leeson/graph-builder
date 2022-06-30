#include <ASTBuilder.h>
#include <sstream>  
#include <iostream>

std::string ptrToStr(const Stmt* ptr){
    std::ostringstream oss;
    oss << ptr;
    std::string ptrStr(oss.str());

    return ptrStr;
}

std::string ptrToStr(const Decl* ptr){
    std::ostringstream oss;
    oss << ptr;
    std::string ptrStr(oss.str());

    return ptrStr;
}

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

            ast.add_node(sPrimeStr, parentStringRep);
            ast.add_node(sStr, stringRep);
            ast.add_edge(sPrimeStr, sStr);
        }
        else if(parent.get<Decl>()){
            auto dPrime = parent.get<Decl>();

            std::string dPrimeStr = ptrToStr(dPrime);
            std::string sStr = ptrToStr(s);

            ast.add_node(dPrimeStr, dPrime->getDeclKindName());
            ast.add_node(sStr, s->getStmtClassName());
            ast.add_edge(dPrimeStr, sStr);
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

                ast.add_node("typePlaceholder"+std::to_string(placeholderVal), "array");
                ast.add_node(sStr, stringRep);
                ast.add_edge("typePlaceholder"+std::to_string(placeholderVal), sStr);
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
        if(d->getDecl()->isImplicit() || ast.nodePtrToNum.find(s1) == ast.nodePtrToNum.end()){
            std::string sStr = ptrToStr(s);
            std::string dStr = ptrToStr(d->getDecl());             
            
            ast.add_node(sStr, s->getStmtClassName());
            ast.add_node(dStr, d->getDecl()->getDeclKindName());
            ast.add_edge(sStr, dStr);
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

            ast.add_node(sPrimeStr, sPrime->getStmtClassName());
            ast.add_node(dStr, d->getDeclKindName());
            ast.add_edge(sPrimeStr, dStr);
        }
        else if(parent.get<Decl>()){
            auto dPrime = parent.get<Decl>();
            std::string dPrimeStr = ptrToStr(dPrime);
            std::string dStr = ptrToStr(d);
            std::string paramStr;
            if(isa<FunctionDecl>(d)){
                FunctionDecl* f = cast<FunctionDecl>(d);
                if(f->getNameInfo().getAsString().find(inputString) != std::string::npos){
                    ast.add_node(dPrimeStr, dPrime->getDeclKindName());
                    ast.add_node(dStr, "input " + f->getReturnType().getAsString());
                    ast.add_edge(dPrimeStr, dStr);
                }
                else if(f->isMain()){
                    ast.add_node(dPrimeStr, dPrime->getDeclKindName());
                    ast.add_node(dStr, "main");
                    ast.add_edge(dPrimeStr, dStr);
                }
                else{
                    ast.add_node(dPrimeStr, dPrime->getDeclKindName());
                    ast.add_node(dStr, d->getDeclKindName());
                    ast.add_edge(dPrimeStr, dStr);
                }
                for(auto param : f->parameters()){
                    paramStr = ptrToStr(param);
                    ast.add_node(paramStr, param->getDeclKindName());
                    ast.add_edge(dStr, paramStr);
                }
            }
            else{
                ast.add_node(dPrimeStr, dPrime->getDeclKindName());
                ast.add_node(dStr, d->getDeclKindName());
                ast.add_edge(dPrimeStr, dStr);
            }
        }
    }

    if(isa<VarDecl>(d)){
        VarDecl* v = cast<VarDecl>(d);
        std::string dStr = ptrToStr(d); 

        if(v->getType().getTypePtr()->isBuiltinType()){
            ast.add_node(dStr, d->getDeclKindName());
            ast.add_node("typePlaceholder" + std::to_string(placeholderVal), v->getType().getDesugaredType(*Context).getAsString());
            ast.add_edge(dStr, "typePlaceholder" + std::to_string(placeholderVal));
            placeholderVal++;
        }
        else if(v->getType().getTypePtr()->isStructureType()){
            ast.add_node(dStr, d->getDeclKindName());
            ast.add_node("typePlaceholder" + std::to_string(placeholderVal), "struct");
            ast.add_edge(dStr, "typePlaceholder" + std::to_string(placeholderVal));
            if(v->hasInit()){
                std::string vStr = ptrToStr(v->getInit());
                ast.add_node(vStr, v->getInit()->getStmtClassName());
                ast.add_edge("typePlaceholder" + std::to_string(placeholderVal), vStr);
            }
            placeholderVal++;
        }
        else if(v->getType().getTypePtr()->isArrayType()){
            ast.add_node(dStr, d->getDeclKindName());
            ast.add_node("typePlaceholder" + std::to_string(placeholderVal), "array");
            ast.add_edge(dStr, "typePlaceholder" + std::to_string(placeholderVal));
            if(v->hasInit()){
                std::string vStr = ptrToStr(v->getInit());
                ast.add_node(vStr, v->getInit()->getStmtClassName());
                ast.add_edge("typePlaceholder" + std::to_string(placeholderVal),vStr);
            }
            placeholderVal++;
        }
        else if(v->getType().getTypePtr()->isPointerType()){
            ast.add_node(dStr, d->getDeclKindName());
            ast.add_node("typePlaceholder" + std::to_string(placeholderVal), "pointer");
            ast.add_edge(dStr, "typePlaceholder" + std::to_string(placeholderVal));
            placeholderVal++;
        }
        else{
            ast.add_node(dStr, d->getDeclKindName());
            ast.add_node("typePlaceholder" + std::to_string(placeholderVal), "otherType");
            ast.add_edge(dStr, "typePlaceholder" + std::to_string(placeholderVal));
            placeholderVal++;
        }
    }

    return true;
}

void ASTBuilder::serializeGraph(){
    for(auto node : ast.get_nodes()){
        std::cout<<node<<std::endl;
    }
    for(auto [from, to] : zip(ast.get_outEdges(),ast.get_inEdges())){
        std::cout<<"From:"<<from<<" To:"<< to<<std::endl;
    }
}