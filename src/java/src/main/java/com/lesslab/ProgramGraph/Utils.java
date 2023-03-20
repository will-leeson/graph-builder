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
        Option classPath = new Option("c", "class", true, "The path to the Class");

        projectPath.setRequired(true);
        classPath.setRequired(true);

        options.addOption(projectPath);
        options.addOption(classPath);
        

        CommandLineParser parser = new DefaultParser();
        CommandLine cmd = parser.parse(options, args);

        return cmd;
    } 
}
