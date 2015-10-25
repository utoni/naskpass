CFLAGS = $(shell ncurses5-config --cflags) -Wall -Wundef -Wshadow -D_GNU_SOURCE=1 -fPIC -fomit-frame-pointer -fno-inline -fstrength-reduce -frerun-cse-after-loop -frerun-loop-opt -fexpensive-optimizations -fstrict-aliasing -Os -MD -MP
DBGFLAGS = -g
LDFLAGS = $(shell ncurses5-config --libs) -pthread -lrt
CC = gcc
INSTALL = install
STRIP = strip
VERSION = $(shell if [ -d ./.git ]; then echo -n "git-"; git rev-parse --short HEAD; else echo "1.2a"; fi)
BIN = naskpass
SOURCES = status.c ui_ani.c ui_input.c ui_statusbar.c ui_nwindow.c ui.c ui_elements.c ui_ipc.c main.c
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))
DEPS = $(patsubst %.c,%.d,$(SOURCES))

all: $(OBJECTS) $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) -D_VERSION=\"$(VERSION)\" -c $< -o $@

$(BIN): $(SOURCES)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(BIN)
	$(MAKE) -C tests CC='$(CC)' CFLAGS='$(CFLAGS)' all

strip: $(OBJECTS) $(BIN)
	$(STRIP) $(BIN)

release: all strip

debug:
	$(MAKE) CFLAGS='$(CFLAGS) $(DBGFLAGS)'
	$(MAKE) -C tests CFLAGS='$(CFLAGS) $(DBGFLAGS)'

install:
	$(INSTALL) -D -m 0755 $(BIN) $(DESTDIR)/lib/cryptsetup/naskpass
	$(INSTALL) -D -m 0755 scripts/naskpass.inithook $(DESTDIR)/usr/share/naskpass/naskpass.hook.initramfs
	$(INSTALL) -D -m 0755 scripts/naskpass.initscript $(DESTDIR)/usr/share/naskpass/naskpass.script.initramfs
	$(INSTALL) -D -m 0755 scripts/naskconf $(DESTDIR)/usr/share/naskpass/naskconf

uninstall:
	rm -f $(DESTDIR)/lib/cryptsetup/naskpass
	rm -f $(DESTDIR)/usr/share/initramfs-tools/hooks/naskpass
	rm -f $(DESTDIR)/usr/share/naskpass/naskpass.script.initramfs
	rm -f $(DESTDIR)/usr/share/naskpass/naskconf
	rmdir --ignore-fail-on-non-empty $(DESTDIR)/usr/share/naskpass

clean:
	rm -f $(DEPS)
	rm -f $(OBJECTS)
	rm -f $(BIN)
	$(MAKE) -C tests clean

source:
	-dh_make --createorig -p naskpass_$(VERSION) -s -y

.PHONY: all install clean
