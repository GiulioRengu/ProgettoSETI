CC = gcc
CFLAGS = -Wall -Wextra -g -I./server -I./client -I./server_client

# Directory
SERVER_DIR = server
CLIENT_DIR = client
COMMON_DIR = server_client

# File sorgenti (aggiunto msgparsing.c ai sorgenti di test)
SERVER_SRCS = $(SERVER_DIR)/mainServer.c $(SERVER_DIR)/server.c $(COMMON_DIR)/msgparsing.c $(COMMON_DIR)/net.c
CLIENT_SRCS = $(CLIENT_DIR)/mainClient.c $(COMMON_DIR)/msgparsing.c $(COMMON_DIR)/net.c
TEST_SRCS = test.c $(COMMON_DIR)/msgparsing.c $(COMMON_DIR)/net.c

# File oggetto
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)
TEST_OBJS = $(TEST_SRCS:.c=.o)

# Eseguibili finali
SERVER_BIN = server_exe
CLIENT_BIN = client_exe
TEST_BIN = test_exe

# Regole principali
.PHONY: all clean

all: $(SERVER_BIN) $(CLIENT_BIN) $(TEST_BIN)

# Compilazione degli eseguibili
$(SERVER_BIN): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT_BIN): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_BIN): $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compilazione generica dei file oggetto (.c -> .o)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Pulizia dei file generati
clean:
	rm -f $(SERVER_DIR)/*.o $(CLIENT_DIR)/*.o $(COMMON_DIR)/*.o *.o
	rm -f $(SERVER_BIN) $(CLIENT_BIN) $(TEST_BIN)