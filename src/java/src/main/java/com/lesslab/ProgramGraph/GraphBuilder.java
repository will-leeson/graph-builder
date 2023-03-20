package com.lesslab.ProgramGraph;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang3.tuple.Pair;

import com.lesslab.ProgramGraph.Visitors.GraphStmtVisitor;

import sootup.analysis.interprocedural.icfg.CalleeMethodSignature;
import sootup.analysis.interprocedural.icfg.JimpleBasedInterproceduralCFG;
import sootup.callgraph.CallGraph;
import sootup.callgraph.CallGraphAlgorithm;
import sootup.callgraph.ClassHierarchyAnalysisAlgorithm;
import sootup.core.frontend.ResolveException;
import sootup.core.graph.StmtGraph;
import sootup.core.inputlocation.AnalysisInputLocation;
import sootup.core.jimple.common.stmt.Stmt;
import sootup.core.model.SootClass;
import sootup.core.model.SootMethod;
import sootup.core.signatures.MethodSignature;
import sootup.core.typehierarchy.ViewTypeHierarchy;
import sootup.core.types.ClassType;
import sootup.core.util.DotExporter;
import sootup.core.views.View;
import sootup.java.bytecode.inputlocation.JavaClassPathAnalysisInputLocation;
import sootup.java.core.JavaProject;
import sootup.java.core.JavaSootClass;
import sootup.java.core.JavaSootClassSource;
import sootup.java.core.JavaSootMethod;
import sootup.java.core.language.JavaLanguage;
import sootup.java.core.views.JavaView;
import sootup.java.sourcecode.inputlocation.JavaSourcePathAnalysisInputLocation;

public class GraphBuilder {
    private JavaView view;
    private JavaProject project;

    public GraphBuilder(){

    }

    public GraphBuilder(String ProjectPath){
        AnalysisInputLocation<JavaSootClass> inputLocation =
        new JavaClassPathAnalysisInputLocation(ProjectPath);

        JavaLanguage language = new JavaLanguage(8);
        
        project =
            JavaProject.builder(language)
                .addInputLocation(inputLocation)
                .build();

        view = project.createFullView();
    }

    public void BuildGraph(String ClassPath){
        ClassType classVar = this.project.getIdentifierFactory().getClassType(ClassPath);

        SootClass<JavaSootClassSource> sootClass = (SootClass<JavaSootClassSource>) view.getClass(classVar).get();

        System.out.println(sootClass.getMethods().size());
        for(SootMethod x : sootClass.getMethods()){
            if(x.getName().equals("fromCSV")){
                System.out.println(x.getName());
                JimpleBasedInterproceduralCFG icfg = new JimpleBasedInterproceduralCFG(view, x.getSignature(), true, true);

                System.out.println(icfg.getCallsFromWithin(x));
            }
        }
    }

    public List<StmtGraph> getStmtGraphs(){
        List<StmtGraph> stmtGraphs = new ArrayList <StmtGraph>();

        List<JavaSootClass> classes = new ArrayList<JavaSootClass>(this.view.getClasses());

        GraphStmtVisitor g = new GraphStmtVisitor();
        
        String dotString = "";
        int counter = 0;
        for(JavaSootClass aClass : classes){
            for(JavaSootMethod aMethod : aClass.getMethods()){
                try{
                    for(Stmt s : aMethod.getBody().getStmts()){
                        s.accept(g);
                    }
                }
                catch(ResolveException e){
                    continue;
                }
                try{
                    dotString += DotExporter.buildGraph(aMethod.getBody().getStmtGraph());
                }
                catch(ResolveException e){
                    continue;
                }
                
            }
            counter++;
            if(counter > 5){
                break;
            }
            // break;
        }

        Map<Integer,String> finalNodeToRepMap = new HashMap<Integer,String>();
        finalNodeToRepMap.putAll(g.getHashToString());
        finalNodeToRepMap.putAll(g.getGraphValueVisitor().getHashToString());

        List<Integer> finalOutEdges = g.getOutEdges();
        List<Integer> finalInEdges = g.getInEdges();
        List<Integer> edgeTypes = new ArrayList<Integer>(Collections.nCopies(finalOutEdges.size(), 0));

        finalOutEdges.addAll(g.getGraphValueVisitor().getOutEdges());
        finalInEdges.addAll(g.getGraphValueVisitor().getInEdges());
        edgeTypes.addAll(new ArrayList<Integer>(Collections.nCopies(g.getGraphValueVisitor().getOutEdges().size(), 1)));
    
        
        System.out.println(finalNodeToRepMap.toString());
        System.out.println(finalOutEdges.toString());
        System.out.println(finalInEdges.toString());
        System.out.println(edgeTypes.toString());
        // System.out.println(dotString);
        // System.out.println(dotString.replace("}digraph G {", ""));
        return stmtGraphs;
    }
}
