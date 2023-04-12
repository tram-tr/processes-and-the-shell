CC = gcc
CFLAGS = -Wall -g -std=c99

all: myshell

filecopy: myshell.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f myshell
