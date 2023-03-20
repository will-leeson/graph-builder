package com.lesslab.ProgramGraph.Visitors;

import sootup.core.jimple.visitor.StmtVisitor;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;

import sootup.core.jimple.basic.EquivTo;
import sootup.core.jimple.common.stmt.*;
import sootup.core.jimple.javabytecode.stmt.*;

public class GraphStmtVisitor implements StmtVisitor{
    private LinkedList<Integer> inEdges = new LinkedList<Integer>();
    private LinkedList<Integer> outEdges = new LinkedList<Integer>();

    private GraphValueVisitor valueVisitor = new GraphValueVisitor();

    private Map<Integer,String> hashToStringRep = new HashMap<Integer, String>();

    public LinkedList<Integer> getInEdges(){
        return this.inEdges;
    }

    public LinkedList<Integer> getOutEdges(){
        return this.outEdges;
    }

    public Map<Integer,String> getHashToString(){
        return this.hashToStringRep;
    }

    public GraphValueVisitor getGraphValueVisitor(){
        return this.valueVisitor;
    }

    private void addEdge(EquivTo from, EquivTo to){
        outEdges.add(from.hashCode());
        inEdges.add(to.hashCode());

        if(!hashToStringRep.containsKey(from.hashCode())){
            hashToStringRep.put(from.hashCode(), from.getClass().getSimpleName().toString());
        }
        if(!hashToStringRep.containsKey(to.hashCode())){
            hashToStringRep.put(to.hashCode(), to.getClass().getSimpleName().toString());
        }
    }

    public void caseAssignStmt(JAssignStmt<?, ?> stmt){
        addEdge(stmt, stmt.getLeftOp());
        addEdge(stmt, stmt.getRightOp());

        stmt.getRightOp().accept(valueVisitor);
    }

    public void caseIdentityStmt(JIdentityStmt<?> stmt){
        addEdge(stmt, stmt.getLeftOp());
        addEdge(stmt, stmt.getRightOp());

        stmt.getRightOp().accept(valueVisitor);
    }

    public void caseIfStmt(JIfStmt stmt){
        addEdge(stmt, stmt.getCondition());
        stmt.getCondition().accept(valueVisitor);
    }

    public void caseBreakpointStmt(JBreakpointStmt stmt){}

    public void caseInvokeStmt(JInvokeStmt stmt){}

    public void caseEnterMonitorStmt(JEnterMonitorStmt stmt){}

    public void caseExitMonitorStmt(JExitMonitorStmt stmt){}

    public void caseGotoStmt(JGotoStmt stmt){}

    public void caseNopStmt(JNopStmt stmt){}

    public void caseRetStmt(JRetStmt stmt){}

    public void caseReturnStmt(JReturnStmt stmt){}

    public void caseReturnVoidStmt(JReturnVoidStmt stmt){}

    public void caseSwitchStmt(JSwitchStmt stmt){}

    public void caseThrowStmt(JThrowStmt stmt){}

    public void defaultCaseStmt(Stmt stmt){}
}
