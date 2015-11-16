CFLAGS = -Wall -g
LDFLAGS = -lpthread -lrt
CC = gcc
SOURCES = $(wildcard *.c)
BINARIES = $(patsubst %.c,%,$(SOURCES))
DEPS = $(patsubst %.c,%.d,$(SOURCES))

all: $(BINARIES)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $(patsubst %.c,%,$<)

clean:
	rm -f $(DEPS)
	rm -f $(BINARIES)

run: all
	@echo "* running tests"
	for test in $(patsubst %.c,%,$(SOURCES)); do \
		echo -n "* running $${test}"; \
		./$${test} >/dev/null; \
		if [ $$? -ne 0 ]; then \
			echo " FAILED!"; \
		else \
			echo " OK!"; \
		fi; \
	done

.PHONY: all install clean
