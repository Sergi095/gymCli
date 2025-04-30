CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic
TARGET = gymcli
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = /usr/local/bin
DATADIR = $(HOME)/.local/share/gymcli

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

# Create obj directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files to object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c -o $@ $<

# Clean rule (doesn't touch databases)
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Clean all (including databases)
cleanall: clean
	rm -f $(DB_FILES)

# Install rule
install: $(TARGET)
	@echo "Installing $(TARGET) to $(BINDIR)"
	@sudo mkdir -p $(BINDIR)
	@sudo cp $(TARGET) $(BINDIR)/
	@echo "Creating data directory at $(DATADIR)"
	@mkdir -p $(DATADIR)
	@echo "Moving database files to $(DATADIR)"
	@for db in $(DB_FILES); do \
		if [ -f "$$db" ]; then \
			cp "$$db" "$(DATADIR)/"; \
		fi; \
	done
	@echo "Installation complete. Run 'gymcli' to start the program."

# Uninstall rule
uninstall:
	@echo "Removing $(TARGET) from $(BINDIR)"
	@sudo rm -f $(BINDIR)/$(TARGET)
	@echo "Do you want to remove all data including database files? (y/N)"
	@read -r response; \
	if [ "$$response" = "y" ] || [ "$$response" = "Y" ]; then \
		echo "Removing data directory $(DATADIR)"; \
		rm -rf $(DATADIR); \
		echo "All data has been removed."; \
	else \
		echo "Data preserved in $(DATADIR)"; \
	fi
	@echo "Uninstallation complete."

# Run rule
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean cleanall install uninstall run
