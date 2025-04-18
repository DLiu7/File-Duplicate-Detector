CC = gcc
CFLAGS = -Wall -Wextra -g
LIBS = -lssl -lcrypto

TARGET = detect_dups
SRCS = detect_dups.c
HDRS = detect_dups.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm -f $(TARGET)
