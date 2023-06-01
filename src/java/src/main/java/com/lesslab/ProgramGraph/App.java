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

        boolean bbcfg, scfg, cdg, expr, call,data;
        if(cmd.hasOption("all")){
            bbcfg = true;
            scfg = true;
            cdg = true;
            expr = true;
            call = true;
            data = true;
        }
        else{
            bbcfg = cmd.hasOption("bbcfg");
            scfg = cmd.hasOption("scfg");
            cdg = cmd.hasOption("cdg");
            expr = cmd.hasOption("expr");
            call = cmd.hasOption("call");
            data = cmd.hasOption("data");
        }



        GraphBuilder builder = new GraphBuilder(cmd.getOptionValue("p"), bbcfg, scfg, cdg, expr,call,data);

        // x.BuildGraph(cmd.getOptionValue('c'));

        builder.buildGraphs();

        builder.toJson(cmd.getOptionValue("out"));
        // x.buildCallGraphs();
    }
}
