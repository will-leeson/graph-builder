package com.lesslab.ProgramGraph.Visitors;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;

import javax.annotation.Nonnull;

import sootup.core.jimple.visitor.ValueVisitor;
import sootup.core.types.Type;
import sootup.core.jimple.basic.EquivTo;
import sootup.core.jimple.basic.Immediate;
import sootup.core.jimple.basic.Local;
import sootup.core.jimple.basic.StmtPositionInfo;
import sootup.core.jimple.basic.Value;
import sootup.core.jimple.common.expr.*;
import sootup.core.jimple.common.ref.*;
import sootup.core.jimple.common.constant.*;

public class GraphValueVisitor implements ValueVisitor  {
    private LinkedList<Integer> inEdges = new LinkedList<Integer>();
    private LinkedList<Integer> outEdges = new LinkedList<Integer>();
    private LinkedList<String> edgeTypes = new LinkedList<String>();

    private Map<Integer,String> hashToStringRep = new HashMap<Integer, String>();

    public LinkedList<Integer> getInEdges(){
        return this.inEdges;
    }

    public LinkedList<Integer> getOutEdges(){
        return this.outEdges;
    }

    public LinkedList<String> getEdgeTypes(){
        return this.edgeTypes;
    }

    public Map<Integer,String> getHashToString(){
        return this.hashToStringRep;
    }

    private void addEdge(EquivTo from, EquivTo to){
        outEdges.add(from.hashCode());
        inEdges.add(to.hashCode());
        edgeTypes.add("VALUE");

        if(!hashToStringRep.containsKey(from.hashCode())){
            hashToStringRep.put(from.hashCode(), from.getClass().getSimpleName().toString());
        }
        if(!hashToStringRep.containsKey(to.hashCode())){
            hashToStringRep.put(to.hashCode(), to.getClass().getSimpleName().toString());
        }
    }

    private void addEdge(EquivTo object, Type type){
        outEdges.add(type.hashCode());
        inEdges.add(object.hashCode());
        edgeTypes.add("TYPE");

        if(!hashToStringRep.containsKey(type.hashCode())){
            hashToStringRep.put(type.hashCode(), type.getClass().getSimpleName().toString());
        }
        if(!hashToStringRep.containsKey(object.hashCode())){
            hashToStringRep.put(object.hashCode(), object.getClass().getSimpleName().toString());
        }
    }

    public void caseAddExpr(JAddExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }

    public void caseAndExpr(JAndExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseCmpExpr(JCmpExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseCmpgExpr(JCmpgExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseCmplExpr(JCmplExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseDivExpr(JDivExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseEqExpr(JEqExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseNeExpr(JNeExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseGeExpr(JGeExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseGtExpr(JGtExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseLeExpr(JLeExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseLtExpr(JLtExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseMulExpr(JMulExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseOrExpr(JOrExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseRemExpr(JRemExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseShlExpr(JShlExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseShrExpr(JShrExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseUshrExpr(JUshrExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseSubExpr(JSubExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseXorExpr(JXorExpr expr){
        addEdge(expr, expr.getOp1());
        addEdge(expr, expr.getOp2());

        expr.getOp1().accept(this);
        expr.getOp2().accept(this);
    }
  
    public void caseSpecialInvokeExpr(JSpecialInvokeExpr expr){
        for(Immediate i : expr.getArgs()){
            addEdge(expr, i);
            i.accept(this);
        }
    }
  
    public void caseVirtualInvokeExpr(JVirtualInvokeExpr expr){
        for(Immediate i : expr.getArgs()){
            addEdge(expr, i);
            i.accept(this);
        }
    }
  
    public void caseInterfaceInvokeExpr(JInterfaceInvokeExpr expr){
        for(Immediate i : expr.getArgs()){
            addEdge(expr, i);
            i.accept(this);
        }
    }
  
    public void caseStaticInvokeExpr(JStaticInvokeExpr expr){
        for(Immediate i : expr.getArgs()){
            addEdge(expr, i);
            i.accept(this);
        }
    }
  
    public void caseDynamicInvokeExpr(JDynamicInvokeExpr expr){
        for(Immediate i : expr.getArgs()){
            addEdge(expr, i);
            i.accept(this);
        }
    }
  
    public void caseCastExpr(JCastExpr expr){
        addEdge(expr, expr.getOp());
        addEdge(expr, expr.getType());
    }
  
    public void caseInstanceOfExpr(JInstanceOfExpr expr){
        addEdge(expr, expr.getOp());
        addEdge(expr, expr.getType());
    }
  
    public void caseNewArrayExpr(JNewArrayExpr expr){
        addEdge(expr, expr.getBaseType());
        addEdge(expr, expr.getSize());
    }
  
    public void caseNewMultiArrayExpr(JNewMultiArrayExpr expr){
        addEdge(expr, expr.getBaseType());
        for(Immediate s : expr.getSizes()){
            addEdge(expr, s);
        }
    }
  
    public void caseNewExpr(JNewExpr expr){
        addEdge(expr, expr.getType());
    }
  
    public void caseLengthExpr(JLengthExpr expr){}
  
    public void caseNegExpr(JNegExpr expr){}
  
    public void casePhiExpr(JPhiExpr v){
        for(Local arg : v.getArgs()){
            addEdge(v, arg);
            arg.accept(this);
        }
    }
  
    public void defaultCaseExpr(Expr expr){
        addEdge(expr, expr.getType());
    }

    public void caseStaticFieldRef(JStaticFieldRef ref){
        addEdge(ref, ref.getType());
    }

    public void caseInstanceFieldRef(JInstanceFieldRef ref){
        addEdge(ref, ref.getType());
    }

    public void caseArrayRef(JArrayRef ref){
        addEdge(ref, ref.getIndex().getType());
    }

    public void caseParameterRef(JParameterRef ref){
        addEdge(ref, ref.getType());
    }

    public void caseCaughtExceptionRef(JCaughtExceptionRef ref){}

    public void caseThisRef(JThisRef ref){}

    public void defaultCaseRef(Ref ref){}

    public void caseBooleanConstant(@Nonnull BooleanConstant constant){
        addEdge(constant, constant.getType());
    }

    public void caseDoubleConstant(@Nonnull DoubleConstant constant){
        addEdge(constant, constant.getType());
    }

    public void caseFloatConstant(@Nonnull FloatConstant constant){
        addEdge(constant, constant.getType());
    }

    public void caseIntConstant(@Nonnull IntConstant constant){
        addEdge(constant, constant.getType());
    }

    public void caseLongConstant(@Nonnull LongConstant constant){
        addEdge(constant, constant.getType());
    }

    public void caseNullConstant(@Nonnull NullConstant constant){
        addEdge(constant, constant.getType());
    }

    public void caseStringConstant(@Nonnull StringConstant constant){
        addEdge(constant, constant.getType());
    }

    public void caseEnumConstant(@Nonnull EnumConstant constant){
        addEdge(constant, constant.getType());
    }

    public void caseClassConstant(@Nonnull ClassConstant constant){
        addEdge(constant, constant.getType());
    }

    public void caseMethodHandle(@Nonnull MethodHandle handle){
        addEdge(handle, handle.getType());
    }

    public void caseMethodType(@Nonnull MethodType methodType){
        addEdge(methodType, methodType.getReturnType());
    }

    public void defaultCaseConstant(@Nonnull Constant constant){}

    public void caseLocal(@Nonnull Local local){
        addEdge(local, local.getType());
    }

    public void defaultCaseValue(@Nonnull Value v){
        addEdge(v, v.getType());
    }
}
