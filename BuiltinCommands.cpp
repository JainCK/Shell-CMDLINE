#include "BuiltinCommands.h"
#include "Shell.h"
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <climits>
#include <sys/wait.h>
#include <signal.h>

BuiltinCommands::BuiltinCommands(Shell* shellInstance) : shell(shellInstance) {
    registerCommands();
}

void BuiltinCommands::registerCommands() {
    commands["exit"] = [this](const std::vector<std::string>& args) { exitCommand(args); };
    commands["cd"] = [this](const std::vector<std::string>& args) { cdCommand(args); };
    commands["pwd"] = [this](const std::vector<std::string>& args) { pwdCommand(args); };
    commands["echo"] = [this](const std::vector<std::string>& args) { echoCommand(args); };
    commands["export"] = [this](const std::vector<std::string>& args) { exportCommand(args); };
    commands["unset"] = [this](const std::vector<std::string>& args) { unsetCommand(args); };
    commands["history"] = [this](const std::vector<std::string>& args) { historyCommand(args); };
    commands["help"] = [this](const std::vector<std::string>& args) { helpCommand(args); };
    commands["jobs"] = [this](const std::vector<std::string>& args) { jobsCommand(args); };
    commands["fg"] = [this](const std::vector<std::string>& args) { fgCommand(args); };
}

bool BuiltinCommands::isBuiltin(const std::string& command) const {
    return commands.find(command) != commands.end();
}

bool BuiltinCommands::execute(const std::vector<std::string>& args) {
    if (args.empty()) return false;
    
    auto it = commands.find(args[0]);
    if (it != commands.end()) {
        it->second(args);
        return true;
    }
    
    return false;
}

std::vector<std::string> BuiltinCommands::getAvailableCommands() const {
    std::vector<std::string> commandList;
    for (const auto& pair : commands) {
        commandList.push_back(pair.first);
    }
    return commandList;
}

void BuiltinCommands::exitCommand(const std::vector<std::string>& args) {
    int exitCode = 0;
    
    if (args.size() > 1) {
        try {
            exitCode = std::stoi(args[1]);
        } catch (const std::exception&) {
            std::cerr << "MyShell: exit: numeric argument required\n";
            exitCode = 1;
        }
    }
    
    std::cout << "Exiting MyShell with code " << exitCode << ". Goodbye!\n";
    shell->shutdown();
    exit(exitCode);
}

void BuiltinCommands::cdCommand(const std::vector<std::string>& args) {
    const char* path = nullptr;
    
    if (args.size() == 1) {
        // No argument provided - go to home directory
        path = getenv("HOME");
        if (!path) {
            std::cerr << "MyShell: cd: HOME not set\n";
            return;
        }
    } else if (args.size() == 2) {
        // Handle special cases
        if (args[1] == "~") {
            path = getenv("HOME");
        } else if (args[1] == "-") {
            path = getenv("OLDPWD");
            if (!path) {
                std::cerr << "MyShell: cd: OLDPWD not set\n";
                return;
            }
        } else {
            path = args[1].c_str();
        }
    } else {
        std::cerr << "MyShell: cd: too many arguments\n";
        return;
    }
    
    // Get current directory for OLDPWD
    char oldpwd[PATH_MAX];
    if (getcwd(oldpwd, sizeof(oldpwd)) == nullptr) {
        std::cerr << "MyShell: cd: cannot get current directory\n";
        return;
    }
    
    // Change directory
    if (chdir(path) != 0) {
        std::cerr << "MyShell: cd: cannot change directory to '" << path 
                  << "': " << strerror(errno) << "\n";
        return;
    }
    
    // Set OLDPWD
    setenv("OLDPWD", oldpwd, 1);
    
    // Update PWD
    char newpwd[PATH_MAX];
    if (getcwd(newpwd, sizeof(newpwd)) != nullptr) {
        setenv("PWD", newpwd, 1);
    }
}

void BuiltinCommands::pwdCommand(const std::vector<std::string>& args) {
    (void)args; // Suppress unused parameter warning
    
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::cout << cwd << "\n";
    } else {
        std::cerr << "MyShell: pwd: " << strerror(errno) << "\n";
    }
}

void BuiltinCommands::echoCommand(const std::vector<std::string>& args) {
    bool newline = true;
    size_t start = 1;
    
    // Check for -n flag (no newline)
    if (args.size() > 1 && args[1] == "-n") {
        newline = false;
        start = 2;
    }
    
    // Print arguments
    for (size_t i = start; i < args.size(); i++) {
        if (i > start) std::cout << " ";
        std::cout << args[i];
    }
    
    if (newline) std::cout << "\n";
}

void BuiltinCommands::exportCommand(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        // Show all environment variables
        extern char **environ;
        for (char **env = environ; *env != nullptr; env++) {
            std::cout << *env << "\n";
        }
        return;
    }
    
    for (size_t i = 1; i < args.size(); i++) {
        const std::string& assignment = args[i];
        size_t eq_pos = assignment.find('=');
        
        if (eq_pos != std::string::npos) {
            // Variable assignment: VAR=value
            std::string name = assignment.substr(0, eq_pos);
            std::string value = assignment.substr(eq_pos + 1);
            
            // Set in environment
            setenv(name.c_str(), value.c_str(), 1);
            
            // Also set in shell variables
            shell->getVariables()[name] = value;
        } else {
            // Just export existing variable
            auto& vars = shell->getVariables();
            auto it = vars.find(assignment);
            if (it != vars.end()) {
                setenv(assignment.c_str(), it->second.c_str(), 1);
            } else {
                std::cerr << "MyShell: export: " << assignment << ": not found\n";
            }
        }
    }
}

void BuiltinCommands::unsetCommand(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "MyShell: unset: not enough arguments\n";
        return;
    }
    
    for (size_t i = 1; i < args.size(); i++) {
        const std::string& varName = args[i];
        
        // Remove from environment
        unsetenv(varName.c_str());
        
        // Remove from shell variables
        auto& vars = shell->getVariables();
        vars.erase(varName);
    }
}

void BuiltinCommands::historyCommand(const std::vector<std::string>& args) {
    const auto& history = shell->getHistory();
    
    if (args.size() > 1) {
        try {
            int count = std::stoi(args[1]);
            int start = std::max(0, static_cast<int>(history.size()) - count);
            
            for (int i = start; i < static_cast<int>(history.size()); i++) {
                std::cout << i + 1 << "  " << history[i] << "\n";
            }
        } catch (const std::exception&) {
            std::cerr << "MyShell: history: invalid number\n";
        }
    } else {
        // Show all history
        for (size_t i = 0; i < history.size(); i++) {
            std::cout << i + 1 << "  " << history[i] << "\n";
        }
    }
}

void BuiltinCommands::helpCommand(const std::vector<std::string>& args) {
    (void)args; // Suppress unused parameter warning
    
    std::cout << "MyShell Built-in Commands:\n\n";
    std::cout << "  exit [code]      - Exit the shell with optional exit code\n";
    std::cout << "  cd [directory]   - Change directory (cd ~ for home, cd - for previous)\n";
    std::cout << "  pwd              - Print current working directory\n";
    std::cout << "  echo [-n] [args] - Print arguments (-n: no newline)\n";
    std::cout << "  export [VAR=val] - Set environment variables\n";
    std::cout << "  unset VAR        - Unset environment variables\n";
    std::cout << "  history [n]      - Show command history (last n commands)\n";
    std::cout << "  jobs             - Show background jobs\n";
    std::cout << "  fg [job]         - Bring background job to foreground\n";
    std::cout << "  help             - Show this help message\n\n";
    
    std::cout << "Features:\n";
    std::cout << "  • I/O Redirection: cmd < input.txt > output.txt\n";
    std::cout << "  • Pipes: cmd1 | cmd2\n";
    std::cout << "  • Background: cmd &\n";
    std::cout << "  • Variables: $VAR or ${VAR}\n";
    std::cout << "  • Command History: Use 'history' command\n\n";
}

void BuiltinCommands::jobsCommand(const std::vector<std::string>& args) {
    (void)args; // Suppress unused parameter warning
    
    auto& bgProcesses = shell->getBackgroundProcesses();
    
    if (bgProcesses.empty()) {
        std::cout << "No background jobs\n";
        return;
    }
    
    std::cout << "Background Jobs:\n";
    for (size_t i = 0; i < bgProcesses.size(); i++) {
        pid_t pid = bgProcesses[i];
        
        // Check if process is still running
        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);
        
        if (result == 0) {
            // Still running
            std::cout << "[" << i + 1 << "] " << pid << " Running\n";
        } else {
            // Finished
            std::cout << "[" << i + 1 << "] " << pid << " Done\n";
        }
    }
}

void BuiltinCommands::fgCommand(const std::vector<std::string>& args) {
    auto& bgProcesses = shell->getBackgroundProcesses();
    
    if (bgProcesses.empty()) {
        std::cerr << "MyShell: fg: no background jobs\n";
        return;
    }
    
    pid_t pid;
    
    if (args.size() == 1) {
        // Bring most recent background job to foreground
        pid = bgProcesses.back();
        bgProcesses.pop_back();
    } else {
        try {
            int jobNum = std::stoi(args[1]) - 1; // Convert to 0-based index
            if (jobNum < 0 || jobNum >= static_cast<int>(bgProcesses.size())) {
                std::cerr << "MyShell: fg: job not found\n";
                return;
            }
            
            pid = bgProcesses[jobNum];
            bgProcesses.erase(bgProcesses.begin() + jobNum);
        } catch (const std::exception&) {
            std::cerr << "MyShell: fg: invalid job number\n";
            return;
        }
    }
    
    std::cout << "Bringing process " << pid << " to foreground\n";
    
    // Wait for the process
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        std::cerr << "MyShell: fg: failed to wait for process " << pid 
                  << ": " << strerror(errno) << "\n";
    }
}