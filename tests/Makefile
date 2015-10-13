CFLAGS = -Wall
LDFLAGS = -lpthread
CC = gcc
SOURCES = $(wildcard *.c)
BINARIES = $(patsubst %.c,%,$(SOURCES))

all: $(BINARIES)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $(patsubst %.c,%,$<)

clean:
	rm -f $(BINARIES)

.PHONY: all install clean
