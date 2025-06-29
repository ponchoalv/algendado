CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g -I/opt/homebrew/include
LIBS=-lsqlite3 -lpthread -L/opt/homebrew/lib
SERVER_LIBS=$(LIBS) -lmicrohttpd -lraylib
CLIENT_LIBS=$(LIBS)

# Directories
SRC_DIR=src
BUILD_DIR=build
INCLUDE_DIR=include

# Source files
SERVER_SOURCES=$(SRC_DIR)/server.c $(SRC_DIR)/database.c $(SRC_DIR)/notifications.c $(SRC_DIR)/web_handler.c $(SRC_DIR)/calendar.c $(SRC_DIR)/utils.c
CLIENT_SOURCES=$(SRC_DIR)/client.c $(SRC_DIR)/database.c $(SRC_DIR)/utils.c

# Object files
SERVER_OBJECTS=$(SERVER_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
CLIENT_OBJECTS=$(CLIENT_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Executables
SERVER_TARGET=algen-server
CLIENT_TARGET=algen
NOTIFICATION_TARGET=algen-notify
STACK_TARGET=algen-stack

.PHONY: all clean install

all: $(BUILD_DIR) $(SERVER_TARGET) $(CLIENT_TARGET) $(NOTIFICATION_TARGET) $(STACK_TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(SERVER_TARGET): $(SERVER_OBJECTS)
	$(CC) $(SERVER_OBJECTS) -o $@ $(SERVER_LIBS)

$(CLIENT_TARGET): $(CLIENT_OBJECTS)
	$(CC) $(CLIENT_OBJECTS) -o $@ $(CLIENT_LIBS)

$(NOTIFICATION_TARGET): $(BUILD_DIR)/notification_popup.o $(BUILD_DIR)/notifications.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/database.o
	$(CC) $(BUILD_DIR)/notification_popup.o $(BUILD_DIR)/notifications.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/database.o -o $@ $(SERVER_LIBS)

$(STACK_TARGET): $(BUILD_DIR)/notification_stack.o $(BUILD_DIR)/notifications.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/database.o
	$(CC) $(BUILD_DIR)/notification_stack.o $(BUILD_DIR)/notifications.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/database.o -o $@ $(SERVER_LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(SERVER_TARGET) $(CLIENT_TARGET) $(NOTIFICATION_TARGET) $(STACK_TARGET)

install: all
	cp $(CLIENT_TARGET) /usr/local/bin/
	cp $(SERVER_TARGET) /usr/local/bin/

# Dependencies for macOS notifications
install-deps:
	@echo "Installing dependencies..."
	@echo "Make sure you have:"
	@echo "- sqlite3 development libraries"
	@echo "- libmicrohttpd (brew install libmicrohttpd)"
	@echo "Run: brew install sqlite libmicrohttpd"
