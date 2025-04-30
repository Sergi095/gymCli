# Detect OS
ifeq ($(OS),Windows_NT)
    DETECTED_OS := Windows
    EXE_EXT := .exe
    RM_CMD := del /Q
    MKDIR_CMD := mkdir
    CP_CMD := copy
    INSTALL_DIR := $(APPDATA)\GymCli
    SYSTEM_BIN := C:\Windows\System32
else
    DETECTED_OS := $(shell uname -s)
    EXE_EXT :=
    RM_CMD := rm -rf
    MKDIR_CMD := mkdir -p
    CP_CMD := cp
    INSTALL_DIR := $(HOME)/.local/share/gymcli
    SYSTEM_BIN := /usr/local/bin
endif

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic
TARGET = gymcli$(EXE_EXT)
SRCDIR = src
INCDIR = include
OBJDIR = obj

# Database files
DB_FILES = gym_data.db gym_routines.db

# Source files
SRCS = $(wildcard $(SRCDIR)/*.cpp)
# Object files
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
# Header files
DEPS = $(wildcard $(INCDIR)/*.h)

# Main build rule
all: $(OBJDIR) $(TARGET)
	@echo "Build complete for $(DETECTED_OS)"

# Create obj directory
$(OBJDIR):
	$(MKDIR_CMD) $(OBJDIR)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files to object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c -o $@ $<

# Clean rule (doesn't touch databases)
clean:
	$(RM_CMD) $(OBJDIR) $(TARGET)
	@echo "Cleaned build files"

# OS-specific install rules
ifeq ($(DETECTED_OS),Windows)
install: $(TARGET)
	@echo "Installing for Windows..."
	@echo "Creating data directory in %APPDATA%\GymCli"
	@if not exist "$(INSTALL_DIR)" $(MKDIR_CMD) "$(INSTALL_DIR)" 
	@echo "Copying executable"
	@$(CP_CMD) $(TARGET) "$(TARGET)"
	@echo "Creating batch file for easy access"
	@echo @echo off > gymcli.bat
	@echo set DATADIR=%%APPDATA%%\GymCli >> gymcli.bat
	@echo if not exist "%%DATADIR%%" mkdir "%%DATADIR%%" >> gymcli.bat
	@echo cd /d "%%DATADIR%%" >> gymcli.bat
	@echo "%%~dp0gymcli.exe" >> gymcli.bat
	@echo "Installation complete. Run 'gymcli.bat' to start the program."
else
install: $(TARGET)
	@echo "Installing for $(DETECTED_OS)..."
	@echo "Installing $(TARGET) to $(SYSTEM_BIN)"
	@sudo mkdir -p $(SYSTEM_BIN)
	@sudo cp $(TARGET) $(SYSTEM_BIN)/
	@echo "Creating data directory at $(INSTALL_DIR)"
	@mkdir -p $(INSTALL_DIR)
	@echo "Moving database files to $(INSTALL_DIR)"
	@for db in $(DB_FILES); do \
		if [ -f "$$db" ]; then \
			cp "$$db" "$(INSTALL_DIR)/"; \
		fi; \
	done
	@echo "Creating launcher script"
	@echo "#!/bin/bash" > gymcli_launcher
	@echo "cd $(INSTALL_DIR)" >> gymcli_launcher
	@echo "$(SYSTEM_BIN)/$(TARGET)" >> gymcli_launcher
	@chmod +x gymcli_launcher
	@sudo cp gymcli_launcher $(SYSTEM_BIN)/gymcli
	@rm gymcli_launcher
	@echo "Installation complete. Run 'gymcli' to start the program."
endif

# OS-specific uninstall rules
ifeq ($(DETECTED_OS),Windows)
uninstall:
	@echo "Uninstalling from Windows..."
	@echo "Removing executable and batch file"
	@del $(TARGET) gymcli.bat
	@echo "Do you want to remove all data including database files? (y/N)"
	@set /p CONFIRM="Enter y to confirm: "
	@if "%CONFIRM%"=="y" (rd /s /q "$(INSTALL_DIR)" & echo "All data removed.") else (echo "Data preserved in %APPDATA%\GymCli")
	@echo "Uninstallation complete."
else
uninstall:
	@echo "Uninstalling from $(DETECTED_OS)..."
	@echo "Removing $(TARGET) from $(SYSTEM_BIN)"
	@sudo rm -f $(SYSTEM_BIN)/$(TARGET) $(SYSTEM_BIN)/gymcli
	@echo "Do you want to remove all data including database files? (y/N)"
	@read -r response; \
	if [ "$$response" = "y" ] || [ "$$response" = "Y" ]; then \
		echo "Removing data directory $(INSTALL_DIR)"; \
		rm -rf $(INSTALL_DIR); \
		echo "All data has been removed."; \
	else \
		echo "Data preserved in $(INSTALL_DIR)"; \
	fi
	@echo "Uninstallation complete."
endif

# Run rule
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean install uninstall run