CC = gcc
SANITIZE_FLAGS = -fsanitize=address
FLAGS = -Wall -Wextra -std=c17 $(SANITIZE_FLAGS)
INCLUDES = ./include
SRC = ./src
BUILD = ./.build
OBJDIR = $(BUILD)/obj
DEPDIR = $(BUILD)/dep
EXEC_NAME = gmail-cli
SOURCES = $(wildcard $(SRC)/*.c)
OBJECTS = $(patsubst $(SRC)/%.c,$(OBJDIR)/%.o,$(SOURCES))
DEPS = $(patsubst $(SRC)/%.c,$(DEPDIR)/%.d,$(SOURCES))

TARGET = $(BUILD)/$(EXEC_NAME)
MKDIR_CMD = @mkdir -p $(OBJDIR) $(DEPDIR)
LDFLAGS = -lcurl -lmagic

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRC)/%.c
	$(MKDIR_CMD)
	$(CC) $(FLAGS) -I$(INCLUDES) -MMD -MP -c $< -o $@ -MF $(DEPDIR)/$*.d

-include $(DEPS)

run: $(TARGET)
	$(TARGET)

clean:
	rm -rf $(BUILD) .cache

