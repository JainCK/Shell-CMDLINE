# Makefile for MyShell

# A modular command-line shell implementation in C++

# Compiler and flags

CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -Wpedantic -g -O2
LDFLAGS =

# Directories

SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Target executable

TARGET = $(BINDIR)/myshell

# Source files

SOURCES = main.cpp \
 Shell.cpp \
 CommandParser.cpp \
 CommandExecutor.cpp \
 BuiltinCommands.cpp \
 IORedirection.cpp

# Object files (derived from source files)

OBJECTS = $(SOURCES:%.cpp=$(OBJDIR)/%.o)

# Default target

.PHONY: all clean install uninstall test debug release help

all: $(TARGET)

# Create target executable

$(TARGET): $(OBJECTS) | $(BINDIR)
$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
@echo "Build complete: $(TARGET)"

# Compile source files to object files

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
$(CXX) $(CXXFLAGS) -c $< -o $@

# Create directories if they don't exist

$(OBJDIR):
mkdir -p $(OBJDIR)

$(BINDIR):
mkdir -p $(BINDIR)

# Debug build with extra debugging symbols

debug: CXXFLAGS += -DDEBUG -g3 -O0
debug: $(TARGET)

# Release build with optimizations

release: CXXFLAGS += -DNDEBUG -O3 -s
release: $(TARGET)

# Clean build artifacts

clean:
rm -rf $(OBJDIR) $(BINDIR)
@echo "Clean complete"

# Install the shell (optional)

install: $(TARGET)
@echo "Installing MyShell..."
sudo cp $(TARGET) /usr/local/bin/myshell
sudo chmod +x /usr/local/bin/myshell
@echo "MyShell installed to /usr/local/bin/myshell"

# Uninstall the shell

uninstall:
sudo rm -f /usr/local/bin/myshell
@echo "MyShell uninstalled"

# Run basic tests

test: $(TARGET)
@echo "Running basic tests..."
@echo "Testing built-in commands..."
@echo "pwd" | $(TARGET)
@echo "echo Hello World" | $(TARGET)
@echo "help" | $(TARGET)
@echo "exit" | $(TARGET)
@echo "Basic tests completed"

# Show help information

help:
@echo "MyShell Makefile"
@echo ""
@echo "Available targets:"
@echo " all - Build the shell (default)"
@echo " debug - Build with debug symbols"
@echo " release - Build optimized release version"
@echo " clean - Remove build artifacts"
@echo " install - Install shell to /usr/local/bin"
@echo " uninstall- Remove installed shell"
@echo " test - Run basic functionality tests"
@echo " help - Show this help message"
@echo ""
@echo "Usage:"
@echo " make # Build the shell"
@echo " make debug # Build debug version"
@echo " make test # Run tests"
@echo " make clean # Clean build files"

# Dependency tracking (automatically generated)

-include $(OBJECTS:.o=.d)

# Generate dependency files

$(OBJDIR)/%.d: %.cpp | $(OBJDIR)
	@$(CXX) $(CXXFLAGS) -MM -MT $(@:.d=.o) $< > $@
