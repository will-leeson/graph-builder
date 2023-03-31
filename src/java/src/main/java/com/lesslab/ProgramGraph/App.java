package com.lesslab.ProgramGraph;

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.ParseException;

public class App 
{
    public static void main( String[] args )
    {      
        CommandLine cmd;  
        try{
            cmd = Utils.getOptions(args);
        }
        catch(ParseException p){
            cmd = null;
            System.err.println(p.getMessage());
            p.printStackTrace();
            System.exit(1);
        }

        boolean bbcfg, scfg, expr, call;
        if(cmd.hasOption("all")){
            bbcfg = true;
            scfg = true;
            expr = true;
            call = true;
        }
        else{
            bbcfg = cmd.hasOption("bbcfg");
            scfg = cmd.hasOption("scfg");
            expr = cmd.hasOption("expr");
            call = cmd.hasOption("call");
        }



        GraphBuilder builder = new GraphBuilder(cmd.getOptionValue("p"), bbcfg, scfg,expr,call);

        // x.BuildGraph(cmd.getOptionValue('c'));

        builder.buildGraphs();

        builder.toJson(cmd.getOptionValue("out"));
        // x.buildCallGraphs();
    }
}
