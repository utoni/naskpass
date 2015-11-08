CFLAGS = -Wall
LDFLAGS = -lpthread -lrt
CC = gcc
SOURCES = $(wildcard *.c)
BINARIES = $(patsubst %.c,%,$(SOURCES))
DEPS = $(patsubst %.c,%.d,$(SOURCES))

all: $(BINARIES)

semtest2:
	$(CC) $(CFLAGS) $(LDFLAGS) -I ../src ../src/ui_ipc.o semtest2.c -o semtest2

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $(patsubst %.c,%,$<)

clean:
	rm -f $(DEPS)
	rm -f $(BINARIES)

.PHONY: all install clean
