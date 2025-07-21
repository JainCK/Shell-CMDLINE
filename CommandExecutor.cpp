#include "CommandExecutor.h"
#include "IORedirection.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <cstring>
#include <algorithm>

CommandExecutor::CommandExecutor(std::vector<pid_t>* bgProcesses) 
    : backgroundProcesses(bgProcesses), ioHandler(nullptr) {}

void CommandExecutor::setIOHandler(IORedirection* handler) {
    ioHandler = handler;
}

void CommandExecutor::executeSimpleCommand(const std::vector<std::string>& args) {
    if (args.empty()) return;
    
    // Convert string vector to char* array for execvp
    std::vector<char*> c_args;
    for (const std::string& arg : args) {
        c_args.push_back(const_cast<char*>(arg.c_str()));
    }
    c_args.push_back(nullptr);
    
    // Execute the command
    execvp(c_args[0], c_args.data());
    
    // If we reach here, execvp failed
    std::cerr << "MyShell Error: Command not found or failed to execute '" 
              << c_args[0] << "' (" << strerror(errno) << ")\n";
    exit(EXIT_FAILURE);
}

void CommandExecutor::execute(const ParsedCommand& cmd) {
    if (cmd.args.empty()) return;
    
    // Handle piped commands
    if (cmd.hasPipe) {
        executeWithPipe(cmd);
        return;
    }
    
    // Fork a new process for the command
    pid_t pid = fork();
    
    if (pid == -1) {
        std::cerr << "MyShell Error: Failed to fork process (" 
                  << strerror(errno) << ")\n";
        return;
    }
    
    if (pid == 0) {
        // Child process
        
        // Handle input redirection
        if (!cmd.inputFile.empty()) {
            if (!ioHandler->setupInputRedirection(cmd.inputFile)) {
                exit(EXIT_FAILURE);
            }
        }
        
        // Handle output redirection
        if (!cmd.outputFile.empty()) {
            if (!ioHandler->setupOutputRedirection(cmd.outputFile, cmd.appendOutput)) {
                exit(EXIT_FAILURE);
            }
        }
        
        // Execute the command
        executeSimpleCommand(cmd.args);
    } else {
        // Parent process
        if (cmd.background) {
            // Add to background processes list
            backgroundProcesses->push_back(pid);
            std::cout << "[Background] Process " << pid << " started: ";
            for (const auto& arg : cmd.args) {
                std::cout << arg << " ";
            }
            std::cout << "\n";
        } else {
            // Wait for foreground process to complete
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                std::cerr << "MyShell Error: waitpid failed (" 
                          << strerror(errno) << ")\n";
            } else {
                // Optionally check exit status
                if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                    // Command exited with non-zero status
                    // Could print this for debugging, but usually not needed
                }
            }
        }
    }
}

void CommandExecutor::executeWithPipe(const ParsedCommand& cmd) {
    if (!ioHandler || cmd.args.empty() || cmd.pipeCommand.empty()) {
        std::cerr << "MyShell Error: Invalid pipe command\n";
        return;
    }
    
    int pipefd[2];
    if (!ioHandler->createPipe(pipefd)) {
        return;
    }
    
    // Fork first process (writer)
    pid_t pid1 = fork();
    if (pid1 == -1) {
        std::cerr << "MyShell Error: Failed to fork first process (" 
                  << strerror(errno) << ")\n";
        ioHandler->closePipe(pipefd);
        return;
    }
    
    if (pid1 == 0) {
        // First child process (writer)
        ioHandler->setupPipe(pipefd, true); // Setup as writer
        
        // Handle input redirection for first command
        if (!cmd.inputFile.empty()) {
            if (!ioHandler->setupInputRedirection(cmd.inputFile)) {
                exit(EXIT_FAILURE);
            }
        }
        
        executeSimpleCommand(cmd.args);
    }
    
    // Fork second process (reader)
    pid_t pid2 = fork();
    if (pid2 == -1) {
        std::cerr << "MyShell Error: Failed to fork second process (" 
                  << strerror(errno) << ")\n";
        ioHandler->closePipe(pipefd);
        return;
    }
    
    if (pid2 == 0) {
        // Second child process (reader)
        ioHandler->setupPipe(pipefd, false); // Setup as reader
        
        // Handle output redirection for second command
        if (!cmd.outputFile.empty()) {
            if (!ioHandler->setupOutputRedirection(cmd.outputFile, cmd.appendOutput)) {
                exit(EXIT_FAILURE);
            }
        }
        
        executeSimpleCommand(cmd.pipeCommand);
    }
    
    // Parent process
    ioHandler->closePipe(pipefd);
    
    if (cmd.background) {
        // Add both processes to background list
        backgroundProcesses->push_back(pid1);
        backgroundProcesses->push_back(pid2);
        std::cout << "[Background] Pipe processes " << pid1 << " | " << pid2 << " started\n";
    } else {
        // Wait for both processes
        int status1, status2;
        waitpid(pid1, &status1, 0);
        waitpid(pid2, &status2, 0);
    }
}

void CommandExecutor::cleanupBackgroundProcesses() {
    auto it = backgroundProcesses->begin();
    while (it != backgroundProcesses->end()) {
        int status;
        pid_t result = waitpid(*it, &status, WNOHANG);
        
        if (result > 0) {
            std::cout << "[Background] Process " << *it << " completed";
            if (WIFEXITED(status)) {
                std::cout << " (exit status: " << WEXITSTATUS(status) << ")";
            }
            std::cout << "\n";
            it = backgroundProcesses->erase(it);
        } else if (result == -1) {
            // Process doesn't exist
            it = backgroundProcesses->erase(it);
        } else {
            ++it;
        }
    }
}