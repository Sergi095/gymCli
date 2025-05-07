# Detect OS
OS_NAME := $(shell uname 2>/dev/null || echo Windows)
ifeq ($(OS_NAME),Windows)
    DETECTED_OS := Windows
    EXE_EXT := .exe
    RM := del /Q
    MKDIR := mkdir
    CP := copy
    INSTALL_DIR := $(APPDATA)\GymCli
    SYSTEM_BIN := C:\Windows\System32
else
    DETECTED_OS := $(OS_NAME)
    EXE_EXT :=
    RM := rm -rf
    MKDIR := mkdir -p
    CP := cp
    INSTALL_DIR := $(HOME)/.local/share/gymcli
    SYSTEM_BIN := /usr/local/bin
endif

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic
TARGET = gymcli$(EXE_EXT)
BIN_TARGET = gymcli.bin$(EXE_EXT)
SRCDIR = src
INCDIR = include
OBJDIR = obj
DB_FILES = gym_data.db gym_routines.db

SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
DEPS = $(wildcard $(INCDIR)/*.h)

all: $(OBJDIR) $(TARGET)
	@echo "Build complete for $(DETECTED_OS)"

$(OBJDIR):
	$(MKDIR) $(OBJDIR)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c -o $@ $<

clean:
	$(RM) $(OBJDIR) $(TARGET)
	@echo "Cleaned build files"

install: $(TARGET)
ifeq ($(DETECTED_OS),Windows)
	@echo Installing on Windows...
	@if not exist "$(INSTALL_DIR)" $(MKDIR) "$(INSTALL_DIR)"
	@$(CP) $(TARGET) "$(SYSTEM_BIN)\$(BIN_TARGET)"
	@echo @echo off > "$(SYSTEM_BIN)\gymcli.bat"
	@echo set DATADIR=%%APPDATA%%\GymCli >> "$(SYSTEM_BIN)\gymcli.bat"
	@echo if not exist "%%DATADIR%%" mkdir "%%DATADIR%%" >> "$(SYSTEM_BIN)\gymcli.bat"
	@echo cd /d "%%DATADIR%%" >> "$(SYSTEM_BIN)\gymcli.bat"
	@echo $(SYSTEM_BIN)\$(BIN_TARGET) >> "$(SYSTEM_BIN)\gymcli.bat"
	@echo Installation complete.
else
	@echo Installing on $(DETECTED_OS)...
	sudo mkdir -p "$(SYSTEM_BIN)"
	sudo cp "$(TARGET)" "$(SYSTEM_BIN)/$(BIN_TARGET)"
	mkdir -p "$(INSTALL_DIR)"
	@for db in $(DB_FILES); do \
		if [ -f "$$db" ]; then cp "$$db" "$(INSTALL_DIR)/"; fi; \
	done
	echo '#!/bin/bash' > gymcli_launcher
	echo 'DATA_DIR="$$HOME/.local/share/gymcli"' >> gymcli_launcher
	echo 'cd "$$DATA_DIR" || exit 1' >> gymcli_launcher
	echo 'exec "$(SYSTEM_BIN)/$(BIN_TARGET)"' >> gymcli_launcher
	chmod +x gymcli_launcher
	sudo cp gymcli_launcher "$(SYSTEM_BIN)/gymcli"
	rm gymcli_launcher
	@echo Installation complete.
endif

uninstall:
ifeq ($(DETECTED_OS),Windows)
	@echo Uninstalling from Windows...
	@del "$(SYSTEM_BIN)\$(BIN_TARGET)" "$(SYSTEM_BIN)\gymcli.bat"
else
	@echo Uninstalling from $(DETECTED_OS)...
	sudo rm -f "$(SYSTEM_BIN)/$(BIN_TARGET)" "$(SYSTEM_BIN)/gymcli"
	@echo "removing data: sudo rm -rf $(INSTALL_DIR)"
	sudo rm -rf $(INSTALL_DIR)
endif

run: $(TARGET)
	./$(TARGET)

devinstall: $(TARGET)
ifeq ($(DETECTED_OS),Windows)
	@if not exist "test_install" mkdir test_install
	@$(CP) $(TARGET) "test_install\$(BIN_TARGET)"
	@echo @echo off > "test_install\gymcli.bat"
	@echo set DATADIR=test_data >> "test_install\gymcli.bat"
	@echo if not exist "%%DATADIR%%" mkdir "%%DATADIR%%" >> "test_install\gymcli.bat"
	@echo cd /d "%%DATADIR%%" >> "test_install\gymcli.bat"
	@echo ..\$(BIN_TARGET) >> "test_install\gymcli.bat"
else
	mkdir -p test_install/bin test_install/data
	cp $(TARGET) test_install/bin/$(BIN_TARGET)
	echo '#!/bin/bash' > test_install/bin/gymcli
	echo 'SCRIPT_DIR="$$(cd "$$(dirname "$$0")" && pwd)"' >> test_install/bin/gymcli
	echo 'DATA_DIR="$$SCRIPT_DIR/../data"' >> test_install/bin/gymcli
	echo 'cd "$$DATA_DIR" && exec "$$SCRIPT_DIR/$(BIN_TARGET)"' >> test_install/bin/gymcli
	chmod +x test_install/bin/gymcli
endif
	@echo Test environment created: ./test_install/bin/gymcli

.PHONY: all clean install uninstall run devinstall

