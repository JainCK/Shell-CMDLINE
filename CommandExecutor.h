#ifndef COMMAND_EXECUTOR_H
#define COMMAND_EXECUTOR_H

#include "CommandParser.h"
#include <vector>
#include <sys/types.h>

class IORedirection; // Forward declaration

/**
 * CommandExecutor handles the execution of external commands
 * Responsibilities:
 * - Fork processes for command execution
 * - Handle simple command execution
 * - Handle piped command execution
 * - Manage background processes
 * - Coordinate with IORedirection for file operations
 */
class CommandExecutor {
private:
    std::vector<pid_t>* backgroundProcesses;
    IORedirection* ioHandler;
    
    void executeSimpleCommand(const std::vector<std::string>& args);
    
public:
    CommandExecutor(std::vector<pid_t>* bgProcesses);
    
    /**
     * Set the IO redirection handler
     * @param handler Pointer to IORedirection instance
     */
    void setIOHandler(IORedirection* handler);
    
    /**
     * Execute a parsed command with all its features
     * @param cmd The parsed command structure
     */
    void execute(const ParsedCommand& cmd);
    
    /**
     * Execute a command with pipe support
     * @param cmd The parsed command with pipe information
     */
    void executeWithPipe(const ParsedCommand& cmd);
    
    /**
     * Clean up finished background processes
     */
    void cleanupBackgroundProcesses();
};

#endif // COMMAND_EXECUTOR_H