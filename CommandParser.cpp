#include "CommandParser.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>

CommandParser::CommandParser(const std::map<std::string, std::string>* variables) 
    : shellVariables(variables) {}

std::vector<std::string> CommandParser::tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::istringstream iss(input);
    std::string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string CommandParser::expandVariables(const std::string& input) {
    std::string result = input;
    size_t pos = 0;
    
    while ((pos = result.find('$', pos)) != std::string::npos) {
        size_t start = pos + 1;
        size_t end = start;
        
        // Find the end of the variable name
        while (end < result.length() && 
               (std::isalnum(result[end]) || result[end] == '_')) {
            end++;
        }
        
        if (end > start) {
            std::string varName = result.substr(start, end - start);
            std::string varValue;
            
            // Check environment variables first
            const char* envValue = getenv(varName.c_str());
            if (envValue) {
                varValue = envValue;
            } else {
                // Check shell variables
                auto it = shellVariables->find(varName);
                if (it != shellVariables->end()) {
                    varValue = it->second;
                }
                // If variable not found, leave it as empty string
            }
            
            result.replace(pos, end - pos, varValue);
            pos += varValue.length();
        } else {
            pos++;
        }
    }
    
    return result;
}

ParsedCommand CommandParser::parse(const std::string& commandLine) {
    ParsedCommand cmd;
    
    // Expand variables first
    std::string expanded = expandVariables(commandLine);
    
    // Tokenize the expanded command
    std::vector<std::string> tokens = tokenize(expanded);
    
    // Parse tokens for special operators
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i] == "<" && i + 1 < tokens.size()) {
            // Input redirection
            cmd.inputFile = tokens[++i];
        } else if (tokens[i] == ">" && i + 1 < tokens.size()) {
            // Output redirection (overwrite)
            cmd.outputFile = tokens[++i];
            cmd.appendOutput = false;
        } else if (tokens[i] == ">>" && i + 1 < tokens.size()) {
            // Output redirection (append)
            cmd.outputFile = tokens[++i];
            cmd.appendOutput = true;
        } else if (tokens[i] == "|" && i + 1 < tokens.size()) {
            // Pipe - everything after | goes to pipe command
            cmd.hasPipe = true;
            for (size_t j = i + 1; j < tokens.size(); j++) {
                if (tokens[j] == "&") {
                    cmd.background = true;
                } else {
                    cmd.pipeCommand.push_back(tokens[j]);
                }
            }
            break; // Stop processing after pipe
        } else if (tokens[i] == "&") {
            // Background execution
            cmd.background = true;
        } else {
            // Regular argument
            cmd.args.push_back(tokens[i]);
        }
    }
    
    return cmd;
}

bool CommandParser::isEmpty(const std::string& input) {
    return std::all_of(input.begin(), input.end(), 
                      [](char c) { return std::isspace(c); });
}