package com.lesslab.ProgramGraph.Visitors;

import java.util.stream.Stream;

import javax.annotation.Nonnull;

import sootup.callgraph.ClassHierarchyAnalysisAlgorithm;
import sootup.callgraph.RapidTypeAnalysisAlgorithm;
import sootup.core.jimple.common.expr.AbstractInvokeExpr;
import sootup.core.jimple.common.stmt.Stmt;
import sootup.core.model.SootClass;
import sootup.core.model.SootMethod;
import sootup.core.signatures.MethodSignature;
import sootup.core.typehierarchy.TypeHierarchy;
import sootup.core.views.View;

public class LocalCallGraph extends ClassHierarchyAnalysisAlgorithm{
    public LocalCallGraph(@Nonnull View<? extends SootClass<?>> view, @Nonnull TypeHierarchy typeHierarchy){
        super(view);
    }

    public Stream<MethodSignature> resolveLocalCall(SootMethod sourceMethod, AbstractInvokeExpr invokeExpr){
        return this.resolveCall(sourceMethod, invokeExpr);
    }
}
