#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <string>
#include <vector>
#include <map>
using namespace std;
/**
 * Structure to hold parsed command information
 * Contains all necessary data for command execution including
 * I/O redirection, pipes, and background execution flags
 */
struct ParsedCommand {
    vector<string> args;           // Command and its arguments
    string inputFile;                   // Input redirection file
    string outputFile;                  // Output redirection file
    bool appendOutput;                       // Whether to append (>>) or overwrite (>)
    bool background;                         // Whether to run in background (&)
    bool hasPipe;                           // Whether command has pipe (|)
    vector<string> pipeCommand;    // Command after pipe
    
    ParsedCommand() : appendOutput(false), background(false), hasPipe(false) {}
};

/**
 * CommandParser class handles all command line parsing logic
 * Responsibilities:
 * - Split command line into tokens
 * - Handle variable expansion ($VAR)
 * - Parse I/O redirection operators (<, >, >>)
 * - Parse pipe operators (|)
 * - Parse background execution (&)
 */
class CommandParser {
private:
    const map<string, string>* shellVariables;
    
    vector<string> tokenize(const string& input);
    string expandVariables(const string& input);
    
public:
    CommandParser(const map<string, string>* variables);
    
    /**
     * Parse a command line string into structured command information
     * @param commandLine The raw command line input
     * @return ParsedCommand structure with all parsed information
     */
    ParsedCommand parse(const string& commandLine);
    
    /**
     * Check if input is empty or whitespace only
     * @param input The string to check
     * @return true if empty or whitespace only
     */
    bool isEmpty(const string& input);
};

#endif // COMMAND_PARSER_H