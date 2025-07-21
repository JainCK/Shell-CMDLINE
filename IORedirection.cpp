#include "IORedirection.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

IORedirection::IORedirection() {}

IORedirection::~IORedirection() {}

bool IORedirection::redirectInput(const std::string& filename) {
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cerr << "MyShell Error: Cannot open input file '" << filename 
                  << "': " << strerror(errno) << "\n";
        return false;
    }
    
    if (dup2(fd, STDIN_FILENO) == -1) {
        std::cerr << "MyShell Error: Cannot redirect input: " 
                  << strerror(errno) << "\n";
        close(fd);
        return false;
    }
    
    close(fd);
    return true;
}

bool IORedirection::redirectOutput(const std::string& filename, bool append) {
    int flags = O_WRONLY | O_CREAT;
    if (append) {
        flags |= O_APPEND;
    } else {
        flags |= O_TRUNC;
    }
    
    int fd = open(filename.c_str(), flags, 0644);
    if (fd == -1) {
        std::cerr << "MyShell Error: Cannot open output file '" << filename 
                  << "': " << strerror(errno) << "\n";
        return false;
    }
    
    if (dup2(fd, STDOUT_FILENO) == -1) {
        std::cerr << "MyShell Error: Cannot redirect output: " 
                  << strerror(errno) << "\n";
        close(fd);
        return false;
    }
    
    close(fd);
    return true;
}

bool IORedirection::setupInputRedirection(const std::string& inputFile) {
    if (inputFile.empty()) return true;
    return redirectInput(inputFile);
}

bool IORedirection::setupOutputRedirection(const std::string& outputFile, bool append) {
    if (outputFile.empty()) return true;
    return redirectOutput(outputFile, append);
}

bool IORedirection::createPipe(int pipefd[2]) {
#ifdef _WIN32
    // On Windows, use _pipe instead of pipe
    if (_pipe(pipefd, 4096, _O_BINARY) == -1) {
        std::cerr << "MyShell Error: Failed to create pipe: "
                  << strerror(errno) << "\n";
        return false;
    }
#else
    if (pipe(pipefd) == -1) {
        std::cerr << "MyShell Error: Failed to create pipe: "
                  << strerror(errno) << "\n";
        return false;
    }
#endif
    return true;
}

void IORedirection::setupPipe(int pipefd[2], bool isWriter) {
    if (isWriter) {
        // Writer process: close read end, redirect stdout to write end
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
    } else {
        // Reader process: close write end, redirect stdin from read end
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
    }
}

void IORedirection::closePipe(int pipefd[2]) {
    close(pipefd[0]);
    close(pipefd[1]);
}