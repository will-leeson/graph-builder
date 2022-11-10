#include <utils.h>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>

std::map<std::string, int> tokenToNum = {{"&&", 0}, {"TranslationUnit", 1}, {"Function", 2}, {"unsigned short", 3}, {"input unsigned long long ", 4}, {"volatile unsigned short", 5}, {"unsigned long{", 6}, {"volatile unsigned char", 7}, {"|", 8}, {"void", 9}, {"ParenExpr", 10}, {"IfStmt", 11}, {"UnaryExprOrTypeTraitExpr", 12}, {"++", 13}, {"char", 14}, {"input short ", 15}, {"OffsetOfExpr", 16}, {"double", 17}, {"input long long ", 18}, {"const double", 19}, {"~", 20}, {"ForStmt", 21}, {"input u32 ", 22}, {"pointer", 23}, {"WhileStmt", 24}, {"CaseStmt", 25}, {"%", 26}, {"input char * ", 27}, {"GotoStmt", 28}, {"-", 29}, {"BinaryConditionalOperator", 30}, {"input u8 ", 31}, {"<<", 32}, {"volatile char", 33}, {"signed char", 34}, {"OpaqueValueExpr", 35}, {"||", 36}, {"CommaOperator", 37}, {"const long", 38}, {"!", 39}, {"input float ", 40}, {"input unsigned short ", 41}, {"ParmVar", 42}, {"Record", 43}, {"DesignatedInitExpr", 44}, {"struct", 45}, {">>=", 46}, {"!=", 47}, {"InitListExpr", 48}, {"CompoundStmt", 49}, {"=", 50}, {"^", 51}, {"const unsigned long", 52}, {"const signed char", 53}, {"unsigned char", 54}, {"unsigned int", 55}, {"volatile _Bool", 56}, {"<=", 57}, {"const unsigned short", 58}, {"CStyleCastExpr", 59}, {"volatile unsigned int", 60}, {"long double", 61}, {"UnaryOperator", 62}, {"const unsigned long long", 63}, {"Empty", 64}, {"float", 65}, {"-=", 66}, {"long long", 67}, {"const short", 68}, {"const unsigned char", 69}, {"const float", 70}, {"BreakStmt", 71}, {"<", 72}, {"input void * ", 73}, {"ChooseExpr", 74}, {"volatile int", 75}, {"/=", 76}, {"CompoundLiteralExpr", 77}, {"input _Bool ", 78}, {">", 79}, {"input char ", 80}, {"unsigned long long", 81}, {"^=", 82}, {"ContinueStmt", 83}, {"volatile unsigned long long", 84}, {"*", 85}, {"input pthread_t ", 86}, {"PredefinedExpr", 87}, {"input uint ", 88}, {"input unsigned long ", 89}, {"StringLiteral", 90}, {"NullStmt", 91}, {"ReturnStmt", 92}, {"input long ", 93}, {"BinaryOperator", 94}, {"CharacterLiteral", 95}, {">>", 96}, {"&", 97}, {"TypeTraitExpr", 98}, {"long", 99}, {"CallExpr", 100}, {"Field", 101}, {"IntegerLiteral", 102}, {"*=", 103}, {"VAArgExpr", 104}, {"ConstantExpr", 105}, {"input sector_t ", 106}, {"+", 107}, {"const char", 108}, {"DefaultStmt", 109}, {"int", 110}, {"_Bool", 111}, {"EnumConstant", 112}, {"input unsigned char ", 113}, {"--", 114}, {"Var", 115}, {"MemberExpr", 116}, {"DeclRefExpr", 117}, {"input size_t ", 118}, {"==", 119}, {"input u16 ", 120}, {"GCCAsmStmt", 121}, {">=", 122}, {"otherType", 123}, {"const long long", 124}, {"StmtExpr", 125}, {"const int", 126}, {"main", 127}, {"volatile unsigned long", 128}, {"SwitchStmt", 129}, {"volatile long", 130}, {"ImplicitCastExpr", 131}, {"+=", 132}, {"input loff_t ", 133}, {"&=", 134}, {"array", 135}, {"ConditionalOperator", 136}, {"Enum", 137}, {"Typedef", 138}, {"LabelStmt", 139}, {"input int ", 140}, {"input double ", 141}, {"input unsigned int ", 142}, {"DeclStmt", 143}, {"FloatingLiteral", 144}, {"const unsigned int", 145}, {"input ulong ", 146}, {"const _Bool", 147}, {"|=", 148}, {"/", 149}, {"DoStmt", 150}, {"input const char * ", 151}, {"ArraySubscriptExpr", 152}, {"short", 153}, {"<<=", 154}};

void graph::add_node(std::string ptrStr, std::string nodeType){
    if(nodeSet.insert(ptrStr).second){
        nodePtrToNum[ptrStr] = nodeSet.size()-1;
        nodePtrToType[ptrStr] = nodeType;
    }
}

void graph::add_edge(std::string fromPtrStr, std::string toPtrStr){
    outEdges.push_back(fromPtrStr);
    inEdges.push_back(toPtrStr);
    edgeAttrs.push_back(attrVal);
}

void graph::add_edge(std::string fromPtrStr, std::string toPtrStr, int attr){
    outEdges.push_back(fromPtrStr);
    inEdges.push_back(toPtrStr);
    edgeAttrs.push_back(attr);
}

std::vector<std::string> graph::get_nodes() { return nodes; }
std::set<std::string> graph::get_nodeSet() { return nodeSet; }
std::vector<std::string> graph::get_outEdges() { return outEdges; }
std::vector<std::string> graph::get_inEdges() { return inEdges; }
std::vector<int> graph::get_edgeAttrs() { return edgeAttrs; }

void graph::set_nodes(std::vector<std::string> x){ nodes = x; }

std::map<std::string, int> graph::getNodePtrToNum(){ return nodePtrToNum; }

void graph::setNodePtrToNum(std::map<std::string, int> x){ nodePtrToNum = x; }

std::map<std::string, std::string> graph::getNodePtrToType(){ return nodePtrToType; }

void graph::setNodePtrToType(std::map<std::string, std::string> x){ nodePtrToType = x; }

void graph::serializeGraph(){
    std::vector <std::string> x(nodeSet.size());
    std::vector <int> y(nodeSet.size());
    for(auto node : nodeSet){
        x[nodePtrToNum[node]] = nodePtrToType[node];
        if(tokenToNum.find(nodePtrToType[node]) != tokenToNum.end()){
            y[nodePtrToNum[node]] = tokenToNum[nodePtrToType[node]];    
        }
        else{
            y[nodePtrToNum[node]] = tokenToNum.size();
        }
    }
    nodes = x;
    nodesSerial = y;

    for(auto edge : inEdges){
        inEdgesSerial.push_back(nodePtrToNum[edge]);
    }
    for(auto edge : outEdges){
        outEdgesSerial.push_back(nodePtrToNum[edge]);
    }
}

void graph::printGraph(){
    std::cout<<"Nodes"<<std::endl;
    for(auto node : nodes){
        std::cout<<node<<std::endl;
    }
    std::cout<<"outEdge"<<std::endl;
    for(auto outEdge : outEdgesSerial){
        std::cout<<outEdge<<std::endl;
    }
    std::cout<<"inEdge"<<std::endl;
    for(auto inEdge : inEdgesSerial){
        std::cout<<inEdge<<std::endl;
    }
    std::cout<<"edgeAttr"<<std::endl;
    for(auto attr : edgeAttrs){
        std::cout<<attr<<std::endl;
    }
}

void graph::toFile(std::string fileName){
    std::ofstream outFile;
    outFile.open(fileName);
    outFile << "{\"nodes\": [";
    int i=0;
    for(; i<nodes.size()-1; i++){
        outFile << "\""<<nodes[i]<<"\"" << ", ";
    }
    outFile << "\""<< nodes[i] << "\"],";

    outFile <<" \"outEdges\": [";
    i=0;
    for(; i<outEdgesSerial.size()-1; i++){
        outFile << outEdgesSerial[i] << ", ";
    }
    outFile << outEdgesSerial[i] << "],";

    outFile <<" \"inEdges\": [";
    i=0;
    for(; i<inEdgesSerial.size()-1; i++){
        outFile << inEdgesSerial[i] << ", ";
    }
    outFile << inEdgesSerial[i] << "],";

    outFile <<" \"edgeAttr\": [";
    i=0;
    for(; i<edgeAttrs.size()-1; i++){
        outFile << edgeAttrs[i] << ", ";
    }
    outFile << edgeAttrs[i] << "]}";

}

void graph::mergeGraph(graph g){
    for(auto node : g.nodeSet){
        add_node(node, g.nodePtrToType[node]);
    }
    for(auto [outEdge, inEdge, attr] : llvm::zip(g.outEdges, g.inEdges, g.edgeAttrs)){
        add_edge(outEdge, inEdge, attr);
    }
}

std::string ptrToStr(const clang::Stmt* ptr){
    std::ostringstream oss;
    oss << ptr;
    std::string ptrStr(oss.str());

    return ptrStr;
}

std::string ptrToStr(const clang::Decl* ptr){
    std::ostringstream oss;
    oss << ptr;
    std::string ptrStr(oss.str());

    return ptrStr;
}

std::vector<int> graph::get_nodesSerial(){
    return nodesSerial;
}

std::vector<int> graph::get_outEdgesSerial(){
    return outEdgesSerial;
}

std::vector<int> graph::get_inEdgesSerial(){
    return inEdgesSerial;
}