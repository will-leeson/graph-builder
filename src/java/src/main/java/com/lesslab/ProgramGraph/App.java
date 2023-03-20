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
        GraphBuilder x = new GraphBuilder(cmd.getOptionValue("p"));

        // x.BuildGraph(cmd.getOptionValue('c'));

        x.getStmtGraphs();
    }
}
