package com.lesslab.ProgramGraph;


import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.DefaultParser;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.ParseException;

public final class Utils {
    public static CommandLine getOptions(String[] args) throws ParseException{
        final Options options = new Options();
        
        Option projectPath = new Option("p", "project", true,  "The path to the Project");
        Option blockCFG = new Option("bbcfg", "Build Basic Block CFG");
        Option stmtCFG = new Option("scfg", "Build Statement CFG");
        Option expr = new Option("expr", "Build Expression Graph");
        Option call = new Option("call", "Build Call Graph");
        Option all = new Option("all", "Build all graphs");
        Option outputFile = new Option("out", true, "File to dump graph to");

        projectPath.setRequired(true);
        outputFile.setRequired(true);

        options.addOption(projectPath);
        options.addOption(call);
        options.addOption(expr);
        options.addOption(stmtCFG);
        options.addOption(blockCFG);
        options.addOption(all);
        options.addOption(outputFile);
        

        CommandLineParser parser = new DefaultParser();
        CommandLine cmd = parser.parse(options, args);

        return cmd;
    } 
}
