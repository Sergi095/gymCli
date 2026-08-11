# Detect OS and Termux before choosing compiler and install paths.
OS_NAME := $(shell uname 2>/dev/null || echo Windows)
IS_TERMUX := 0
ifneq ($(strip $(TERMUX_VERSION)),)
    IS_TERMUX := 1
else ifneq ($(findstring com.termux,$(PREFIX)),)
    IS_TERMUX := 1
endif

ifeq ($(OS_NAME),Windows)
    DETECTED_OS := Windows
    EXE_EXT := .exe
    RM := del /Q
    MKDIR := mkdir
    CP := copy
    INSTALL_DIR := $(APPDATA)\GymCli
    SYSTEM_BIN := C:\Windows\System32
    CXX_DEFAULT := g++
else
    EXE_EXT :=
    RM := rm -rf
    MKDIR := mkdir -p
    CP := cp
    INSTALL_DIR := $(HOME)/.local/share/gymcli
    SH_PATH := $(or $(shell command -v sh 2>/dev/null),/bin/sh)

    ifeq ($(IS_TERMUX),1)
        DETECTED_OS := Termux
        TERMUX_PREFIX := $(if $(strip $(PREFIX)),$(PREFIX),/data/data/com.termux/files/usr)
        SYSTEM_BIN := $(TERMUX_PREFIX)/bin
        INSTALL_PRIVILEGE :=
        CXX_DEFAULT := clang++
    else
        DETECTED_OS := $(OS_NAME)
        SYSTEM_BIN := /usr/local/bin
        CXX_DEFAULT := g++
        ifeq ($(shell id -u 2>/dev/null),0)
            INSTALL_PRIVILEGE :=
        else
            INSTALL_PRIVILEGE := sudo
        endif
    endif
endif

CXX = $(CXX_DEFAULT)
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic
TARGET = gymcli$(EXE_EXT)
BIN_TARGET = gymcli.bin$(EXE_EXT)
SRCDIR = src
INCDIR = include
OBJDIR = obj
DB_FILES = gym_data.db gym_routines.db
TEST_TARGET = $(OBJDIR)/gymcli_tests$(EXE_EXT)
TEST_SRCS = tests/test_core.cpp \
	$(SRCDIR)/Exercise.cpp \
	$(SRCDIR)/GymDatabse.cpp \
	$(SRCDIR)/Utils.cpp \
	$(SRCDIR)/WorkoutRoutine.cpp

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
	$(INSTALL_PRIVILEGE) $(MKDIR) "$(SYSTEM_BIN)"
	$(INSTALL_PRIVILEGE) $(CP) "$(TARGET)" "$(SYSTEM_BIN)/$(BIN_TARGET)"
	mkdir -p "$(INSTALL_DIR)"
	@for db in $(DB_FILES); do \
		if [ -f "$$db" ]; then cp "$$db" "$(INSTALL_DIR)/"; fi; \
	done
	echo '#!$(SH_PATH)' > gymcli_launcher
	echo 'DATA_DIR="$(INSTALL_DIR)"' >> gymcli_launcher
	echo 'mkdir -p "$$DATA_DIR" || exit 1' >> gymcli_launcher
	echo 'cd "$$DATA_DIR" || exit 1' >> gymcli_launcher
	echo 'exec "$(SYSTEM_BIN)/$(BIN_TARGET)" "$$@"' >> gymcli_launcher
	chmod +x gymcli_launcher
	$(INSTALL_PRIVILEGE) $(CP) gymcli_launcher "$(SYSTEM_BIN)/gymcli"
	$(RM) gymcli_launcher
	@echo "Installed gymcli to $(SYSTEM_BIN)/gymcli"
endif

uninstall:
ifeq ($(DETECTED_OS),Windows)
	@echo Uninstalling from Windows...
	@del "$(SYSTEM_BIN)\$(BIN_TARGET)" "$(SYSTEM_BIN)\gymcli.bat"
else
	@echo Uninstalling from $(DETECTED_OS)...
	$(INSTALL_PRIVILEGE) rm -f "$(SYSTEM_BIN)/$(BIN_TARGET)" "$(SYSTEM_BIN)/gymcli"
	@echo "Workout data kept at $(INSTALL_DIR)"
endif

run: $(TARGET)
	./$(TARGET)

$(TEST_TARGET): $(TEST_SRCS) $(DEPS) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -o $@ $(TEST_SRCS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

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
	echo '#!$(SH_PATH)' > test_install/bin/gymcli
	echo 'SCRIPT_DIR="$$(cd "$$(dirname "$$0")" && pwd)"' >> test_install/bin/gymcli
	echo 'DATA_DIR="$$SCRIPT_DIR/../data"' >> test_install/bin/gymcli
	echo 'cd "$$DATA_DIR" && exec "$$SCRIPT_DIR/$(BIN_TARGET)"' >> test_install/bin/gymcli
	chmod +x test_install/bin/gymcli
endif
	@echo Test environment created: ./test_install/bin/gymcli

.PHONY: all clean install uninstall run test devinstall
