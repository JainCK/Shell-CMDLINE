#include "Shell.h"
#include <iostream>
#include <algorithm>
#include <sys/wait.h>
#include <signal.h>

Shell::Shell() : running(true) {
    // Initialize all components
    parser = std::make_unique<CommandParser>(&shellVariables);
    executor = std::make_unique<CommandExecutor>(&backgroundProcesses);
    builtins = std::make_unique<BuiltinCommands>(this);
    ioHandler = std::make_unique<IORedirection>();
    
    // Set up cross-component dependencies
    executor->setIOHandler(ioHandler.get());
    
    // Initialize some default shell variables
    shellVariables["PS1"] = "myshell> ";
    shellVariables["USER"] = getenv("USER") ? getenv("USER") : "unknown";
    
    // Ignore SIGINT for the shell process (Ctrl+C should only affect child processes)
    signal(SIGINT, SIG_IGN);
}

Shell::~Shell() {
    // Clean up any remaining background processes
    cleanupBackgroundProcesses();
}

void Shell::printWelcomeMessage() {
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    Welcome to MyShell v1.0                  ║\n";
    std::cout << "║                                                              ║\n";
    std::cout << "║  Features:                                                   ║\n";
    std::cout << "║  • Built-in commands (cd, pwd, echo, export, etc.)          ║\n";
    std::cout << "║  • I/O redirection (<, >, >>)                               ║\n";
    std::cout << "║  • Pipes (|)                                                 ║\n";
    std::cout << "║  • Background processes (&)                                 ║\n";
    std::cout << "║  • Variable expansion ($VAR)                                ║\n";
    std::cout << "║  • Command history                                          ║\n";
    std::cout << "║                                                              ║\n";
    std::cout << "║  Type 'help' for available commands, 'exit' to quit.        ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
}

void Shell::printPrompt() {
    // Use PS1 variable if set, otherwise default prompt
    auto ps1_it = shellVariables.find("PS1");
    if (ps1_it != shellVariables.end()) {
        std::cout << ps1_it->second;
    } else {
        std::cout << "myshell> ";
    }
    std::cout.flush();
}

void Shell::addToHistory(const std::string& command) {
    if (!command.empty() && command != commandHistory.back()) {
        commandHistory.push_back(command);
        
        // Limit history size to prevent memory bloat
        const size_t MAX_HISTORY = 1000;
        if (commandHistory.size() > MAX_HISTORY) {
            commandHistory.erase(commandHistory.begin());
        }
    }
}

void Shell::cleanupBackgroundProcesses() {
    auto it = backgroundProcesses.begin();
    while (it != backgroundProcesses.end()) {
        int status;
        pid_t result = waitpid(*it, &status, WNOHANG);
        
        if (result > 0) {
            std::cout << "[Background] Process " << *it << " completed\n";
            it = backgroundProcesses.erase(it);
        } else if (result == -1) {
            // Process doesn't exist anymore
            it = backgroundProcesses.erase(it);
        } else {
            ++it;
        }
    }
}

void Shell::run() {
    printWelcomeMessage();
    
    std::string commandLine;
    
    while (running) {
        // Clean up any finished background processes
        cleanupBackgroundProcesses();
        
        printPrompt();
        
        // Read command line
        if (!std::getline(std::cin, commandLine)) {
            // EOF reached (Ctrl+D)
            std::cout << "\nGoodbye!\n";
            break;
        }
        
        // Skip empty commands
        if (parser->isEmpty(commandLine)) {
            continue;
        }
        
        // Add to history
        addToHistory(commandLine);
        
        // Parse the command
        ParsedCommand parsed = parser->parse(commandLine);
        
        if (parsed.args.empty()) {
            continue;
        }
        
        // Check if it's a built-in command
        if (builtins->isBuiltin(parsed.args[0])) {
            builtins->execute(parsed.args);
        } else {
            // Execute external command
            executor->execute(parsed);
        }
    }
}