#include "Shell.h"
#include <iostream>
#include <exception>

/**
 * MyShell - A Simple Command Line Shell
 * 
 * Features:
 * - Execute external commands
 * - Built-in commands (cd, pwd, echo, export, etc.)
 * - I/O redirection (<, >, >>)
 * - Pipes (|)
 * - Background processes (&)
 * - Variable expansion ($VAR)
 * - Command history
 * - Job control (jobs, fg)
 * 
 * Author: Generated with modular design principles
 * Date: 2025
 */

int main() {
    try {
        Shell shell;
        shell.run();
    } catch (const std::exception& e) {
        std::cerr << "MyShell Fatal Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "MyShell Fatal Error: Unknown exception occurred" << std::endl;
        return 1;
    }
    
    return 0;
}