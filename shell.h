#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "CommandParser.h"
#include "CommandExecutor.h"
#include "BuiltinCommands.h"
#include "IORedirection.h"

using namespace std;

/**
 * Main Shell class that orchestrates the entire shell functionality
 * Follows the Single Responsibility Principle by delegating specific tasks
 * to specialized classes
 */
class Shell {
private:
    unique_ptr<CommandParser> parser;
    unique_ptr<CommandExecutor> executor;
    unique_ptr<BuiltinCommands> builtins;
    unique_ptr<IORedirection> ioHandler;
    
    vector<string> commandHistory;
    map<string, string> shellVariables;
    vector<pid_t> backgroundProcesses;
    bool running;
    
    void printWelcomeMessage();
    void printPrompt();
    void addToHistory(const string& command);
    void cleanupBackgroundProcesses();
    
public:
    Shell();
    ~Shell();
    
    // Main shell loop
    void run();
    
    // Getters for child classes to access shell state
    const vector<string>& getHistory() const { return commandHistory; }
    const map<string, string>& getVariables() const { return shellVariables; }
    map<string, string>& getVariables() { return shellVariables; }
    vector<pid_t>& getBackgroundProcesses() { return backgroundProcesses; }
    
    // Control shell execution
    void shutdown() { running = false; }
    bool isRunning() const { return running; }
};

#endif // SHELL_H