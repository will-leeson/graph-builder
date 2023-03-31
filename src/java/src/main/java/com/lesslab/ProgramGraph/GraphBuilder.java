package com.lesslab.ProgramGraph;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import javax.annotation.Nonnull;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.lesslab.ProgramGraph.Visitors.GraphStmtVisitor;
import com.lesslab.ProgramGraph.Visitors.LocalCallGraph;

import sootup.core.graph.BasicBlock;
import sootup.core.graph.StmtGraph;
import sootup.core.inputlocation.AnalysisInputLocation;
import sootup.core.inputlocation.ClassLoadingOptions;
import sootup.core.jimple.basic.EquivTo;
import sootup.core.jimple.basic.StmtPositionInfo;
import sootup.core.jimple.common.stmt.Stmt;
import sootup.core.model.SourceType;
import sootup.core.signatures.MethodSignature;
import sootup.core.typehierarchy.ViewTypeHierarchy;
import sootup.java.bytecode.inputlocation.JavaClassPathAnalysisInputLocation;
import sootup.java.core.JavaProject;
import sootup.java.core.JavaSootClass;
import sootup.java.core.JavaSootMethod;
import sootup.java.core.language.JavaLanguage;
import sootup.java.core.views.JavaView;
import sootup.core.jimple.basic.FullStmtPositionInfo;

public class GraphBuilder {
    private JavaView localView;
    private JavaView globalView;
    private JavaProject localProject;
    private JavaProject globalProject;
    private ViewTypeHierarchy typeHierarchy;

    private Map<Integer,String> nodeToRepMap;
    private Map<Integer,String> nodeToLoc;
    private List<Integer> outEdges;
    private List<Integer> inEdges;
    private List<String> edgeTypes;
    private GraphStmtVisitor graphStmtVisitor;

    private boolean buildBlockCFG;
    private boolean buildStmtCFG;
    private boolean buildExpressionGraph;
    private boolean buildCallGraph;
    private LocalCallGraph cga;

    public GraphBuilder(){
        this.nodeToRepMap = new HashMap<Integer,String>();
        this.outEdges = new ArrayList<Integer>();
        this.inEdges = new ArrayList<Integer>();
        this.edgeTypes = new ArrayList<String>();
        this.graphStmtVisitor = new GraphStmtVisitor();
        this.buildBlockCFG =true;
        this.buildStmtCFG = true;
        this.buildExpressionGraph = true;
        this.buildCallGraph = true;
        this.cga = new LocalCallGraph(globalView, typeHierarchy);
        this.nodeToLoc = new HashMap<Integer,String>();
    }

    public GraphBuilder(String ProjectPath, boolean buildBlockCFG, boolean buildStmtCFG, boolean buildExpressionGraph, boolean buildCallGraph){
        AnalysisInputLocation<JavaSootClass> inputLocation =
        new JavaClassPathAnalysisInputLocation(ProjectPath, SourceType.Library);

        JavaLanguage language = new JavaLanguage(8);
        
        this.localProject =
            JavaProject.builder(language)
                .addInputLocation(inputLocation)
                .build();

        this.globalProject =
            JavaProject.builder(language)
                .addInputLocation(inputLocation)
                .addInputLocation(
                    new JavaClassPathAnalysisInputLocation(
                        System.getProperty("java.home") + "/lib/rt.jar"))
                .build();

        this.localView = 
            localProject.createOnDemandView(
                analysisInputLocation ->
                    new  ClassLoadingOptions() {
                    @Nonnull
                    @Override
                    public List<sootup.core.transform.BodyInterceptor> getBodyInterceptors() {
                        return Collections.emptyList();
                    }
                    });

        localView.getClasses();

        this.globalView = 
            globalProject.createOnDemandView(
                analysisInputLocation ->
                    new ClassLoadingOptions() {
                    @Nonnull
                    @Override
                    public List<sootup.core.transform.BodyInterceptor> getBodyInterceptors() {
                        return Collections.emptyList();
                    }
                    });

        globalView.getClasses();

        this.typeHierarchy = new ViewTypeHierarchy(globalView);

        this.cga = new LocalCallGraph(globalView, typeHierarchy);

        this.nodeToRepMap = new HashMap<Integer,String>();
        this.outEdges = new ArrayList<Integer>();
        this.inEdges = new ArrayList<Integer>();
        this.edgeTypes = new ArrayList<String>();
        this.graphStmtVisitor = new GraphStmtVisitor();
        this.nodeToLoc = new HashMap<Integer,String>();

        this.buildBlockCFG =buildBlockCFG;
        this.buildStmtCFG = buildStmtCFG;
        this.buildExpressionGraph = buildExpressionGraph;
        this.buildCallGraph = buildCallGraph;
    }

    private void addEdge(Object from, Object to, String edgeType){
        outEdges.add(from.hashCode());
        inEdges.add(to.hashCode());
        edgeTypes.add(edgeType);

        if(!nodeToRepMap.containsKey(from.hashCode())){
            nodeToRepMap.put(from.hashCode(), from.getClass().getSimpleName().toString());
        }
        if(!nodeToRepMap.containsKey(to.hashCode())){
            nodeToRepMap.put(to.hashCode(), to.getClass().getSimpleName().toString());
        }
    }

    /*
     * This function is based heavily on the DotExporter from SootUp
     */
    private void addBBSuccessors(BasicBlock<?> block){
        List<? extends BasicBlock<?>> successors = block.getSuccessors();
        if(successors.size() > 0){
            // Stmt tailStmt = block.getTail();

            for(BasicBlock<?> succ : successors){
                addEdge(block, succ, "BBCFG");
            }
        }
    }

    public void buildGraphs(){
        List<JavaSootClass> classes = new ArrayList<JavaSootClass>(this.localView.getClasses());

        for(JavaSootClass aClass : classes){
            for(JavaSootMethod aMethod : aClass.getMethods()){ 
                if(aMethod.hasBody()){
                    StmtGraph<?> graph = aMethod.getBody().getStmtGraph();
                    Collection<? extends BasicBlock<?>> blocks;
                    try {
                        blocks = graph.getBlocksSorted();
                    } catch (Exception e) {
                        blocks = graph.getBlocks();
                    }
                    
                    boolean firstBlock = true;
                    for(BasicBlock<?> block: blocks){
                        if(this.buildBlockCFG){
                            addBBSuccessors(block);
                            if(firstBlock && buildCallGraph){
                                addEdge(aMethod.getSignature(), block, "BBCFG");
                                firstBlock = false;
                            }
                        }
                        boolean first = true;
                        EquivTo outNode = null;
                        EquivTo inNode;
                        for(Stmt s : block.getStmts()){
                            nodeToLoc.put(s.hashCode(), aClass.getName().toString() +"|||" + aMethod.getName().toString() + "|||" +s.getPositionInfo().getStmtPosition().toString());
                            if(this.buildStmtCFG){
                                inNode = s;
                                if(buildBlockCFG){
                                    addEdge(s, block, "StmtToBB");
                                }
                                if(!first){
                                    addEdge(outNode, inNode, "StmtCFG");
                                }
                                else{
                                    first = !first;
                                }
                                outNode = s;
                            }
                            if(this.buildExpressionGraph){
                                s.accept(graphStmtVisitor);
                            }
                            if(this.buildCallGraph && s.containsInvokeExpr()){
                                Stream<MethodSignature> calls= cga.resolveLocalCall(aMethod, s.getInvokeExpr());
                                for(MethodSignature sig : calls.collect(Collectors.toList())){
                                    if(buildStmtCFG || buildExpressionGraph){
                                        if(aClass.toString().contains("test-classes")){
                                            addEdge(s, sig, "Test-SCall");
                                        }
                                        else{
                                            addEdge(s, sig, "SCall");
                                        }
                                    }
                                    if(buildBlockCFG){
                                        if(aClass.toString().contains("test-classes")){
                                            addEdge(block, sig, "Test-BCall");
                                        }
                                        else{
                                            addEdge(block, sig, "BCall");
                                        }
                                    }
                                }
                            }
                        }
                    }
                }      
            }
        }


        if(buildExpressionGraph){
            this.nodeToRepMap.putAll(this.graphStmtVisitor.getHashToString());
            this.outEdges.addAll(this.graphStmtVisitor.getOutEdges());
            this.inEdges.addAll(this.graphStmtVisitor.getInEdges());
            this.edgeTypes.addAll(this.graphStmtVisitor.getEdgeTypes());
            this.nodeToRepMap.putAll(this.graphStmtVisitor.getGraphValueVisitor().getHashToString());
            outEdges.addAll(this.graphStmtVisitor.getGraphValueVisitor().getOutEdges());
            inEdges.addAll(this.graphStmtVisitor.getGraphValueVisitor().getInEdges());
            edgeTypes.addAll(this.graphStmtVisitor.getGraphValueVisitor().getEdgeTypes());
        }
    }

    public void dumpGraph(){
        System.out.println(this.nodeToRepMap.toString());
        System.out.println(this.outEdges.toString());
        System.out.println(this.inEdges.toString());
        System.out.println(this.edgeTypes.toString());
    }

    public void toJson(String fileLoc){
        Gson gson = new GsonBuilder().enableComplexMapKeySerialization().create();

        // String json = gson.toJson(this.outEdges);
        List<String> nodes = new ArrayList<String>(this.nodeToRepMap.size());
        List<String> locs = new ArrayList<String>(this.nodeToLoc.size());
        Map<Integer, Integer> nodeToCounter = new HashMap<Integer, Integer>();
        int counter = 0;
        for(int node : this.nodeToRepMap.keySet()){
            nodes.add(this.nodeToRepMap.get(node));
            nodeToCounter.put(node, counter);
            counter++;
            if(this.nodeToLoc.containsKey(node)){
                locs.add(nodeToLoc.get(node));
            }
            else{
                locs.add("NoPositionInformation");
            }
        }
        
        List<Integer> outEdges = new ArrayList<Integer>(this.outEdges.size());
        List<Integer> inEdges = new ArrayList<Integer>(this.inEdges.size());
        assert(outEdges.size() == inEdges.size());
        for(int i=0; i< this.inEdges.size(); i++){
            outEdges.add(nodeToCounter.get(this.outEdges.get(i)));
            inEdges.add(nodeToCounter.get(this.inEdges.get(i)));
        }
        gson.toJson(inEdges);
        Path file = Paths.get(fileLoc);

        Map<String,Collection> jsonMap = new HashMap<String,Collection>();

        jsonMap.put("nodes", nodes);
        jsonMap.put("outEdges", outEdges);
        jsonMap.put("inEdges", inEdges);
        jsonMap.put("edgeTypes", edgeTypes);
        jsonMap.put("location", locs);

        try{
            Files.write(file, Collections.singletonList(gson.toJson(jsonMap)), StandardCharsets.UTF_8);
        }
        catch(IOException e){
            System.err.println("This file location does not exist: "+fileLoc);
        }
    }
}
