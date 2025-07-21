#ifndef BUILTIN_COMMANDS_H
#define BUILTIN_COMMANDS_H

#include <string>
#include <vector>
#include <map>
#include <functional>

class Shell; // Forward declaration

/**
 * BuiltinCommands handles all shell built-in commands
 * Built-in commands are executed directly by the shell process
 * rather than forking a new process
 * 
 * Supported commands:
 * - exit: Quit the shell
 * - cd: Change directory
 * - pwd: Print working directory
 * - echo: Print arguments
 * - export: Set environment variables
 * - unset: Unset environment variables
 * - history: Show command history
 * - help: Show available commands
 * - jobs: Show background jobs
 * - fg: Bring background job to foreground
 */
class BuiltinCommands {
private:
    Shell* shell;
    std::map<std::string, std::function<void(const std::vector<std::string>&)>> commands;
    
    // Individual command implementations
    void exitCommand(const std::vector<std::string>& args);
    void cdCommand(const std::vector<std::string>& args);
    void pwdCommand(const std::vector<std::string>& args);
    void echoCommand(const std::vector<std::string>& args);
    void exportCommand(const std::vector<std::string>& args);
    void unsetCommand(const std::vector<std::string>& args);
    void historyCommand(const std::vector<std::string>& args);
    void helpCommand(const std::vector<std::string>& args);
    void jobsCommand(const std::vector<std::string>& args);
    void fgCommand(const std::vector<std::string>& args);
    
    void registerCommands();
    
public:
    BuiltinCommands(Shell* shellInstance);
    
    /**
     * Check if a command is a built-in command
     * @param command The command name to check
     * @return true if it's a built-in command
     */
    bool isBuiltin(const std::string& command) const;
    
    /**
     * Execute a built-in command
     * @param args Command arguments (first element is the command name)
     * @return true if command was executed successfully
     */
    bool execute(const std::vector<std::string>& args);
    
    /**
     * Get list of all available built-in commands
     * @return vector of command names
     */
    std::vector<std::string> getAvailableCommands() const;
};

#endif // BUILTIN_COMMANDS_H