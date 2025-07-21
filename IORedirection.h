#ifndef IO_REDIRECTION_H
#define IO_REDIRECTION_H

#include <string>

/**
 * IORedirection handles all file I/O redirection operations
 * Responsibilities:
 * - Handle input redirection (<)
 * - Handle output redirection (>, >>)
 * - Create and manage pipes for inter-process communication
 * - Manage file descriptors safely
 */
class IORedirection {
private:
    /**
     * Redirect standard input from a file
     * @param filename The file to redirect from
     * @return true if successful, false otherwise
     */
    bool redirectInput(const std::string& filename);
    
    /**
     * Redirect standard output to a file
     * @param filename The file to redirect to
     * @param append Whether to append (true) or overwrite (false)
     * @return true if successful, false otherwise
     */
    bool redirectOutput(const std::string& filename, bool append);
    
public:
    IORedirection();
    ~IORedirection();
    
    /**
     * Setup input redirection for a command
     * @param inputFile The file to redirect input from
     * @return true if successful, false if error occurred
     */
    bool setupInputRedirection(const std::string& inputFile);
    
    /**
     * Setup output redirection for a command
     * @param outputFile The file to redirect output to
     * @param append Whether to append to file or overwrite
     * @return true if successful, false if error occurred
     */
    bool setupOutputRedirection(const std::string& outputFile, bool append);
    
    /**
     * Create a pipe for inter-process communication
     * @param pipefd Array to store the pipe file descriptors
     * @return true if successful, false if error occurred
     */
    bool createPipe(int pipefd[2]);
    
    /**
     * Setup pipe for command execution
     * @param pipefd The pipe file descriptors
     * @param isWriter true if this process writes to pipe, false if it reads
     */
    void setupPipe(int pipefd[2], bool isWriter);
    
    /**
     * Close pipe file descriptors
     * @param pipefd The pipe file descriptors to close
     */
    void closePipe(int pipefd[2]);
};

#endif // IO_REDIRECTION_H