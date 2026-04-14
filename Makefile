CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
TARGET = main
SRCS = main.c miniredis.c tcpserver.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS) redislikeinc.h
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
