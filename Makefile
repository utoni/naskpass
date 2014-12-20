CFLAGS ?= $(shell ncurses5-config --cflags) -Wall -D_GNU_SOURCE=1 -g
LDFLAGS ?= $(shell ncurses5-config --libs) -pthread
CC := gcc
INSTALL ?= install
VERSION = $(shell if [ -d ./.git ]; then echo -n "git-"; git rev-parse --short HEAD; else echo "1.1a"; fi)
BIN = naskpass
SOURCES = status.c ui_ani.c ui_input.c ui_statusbar.c ui.c main.c

all: $(BIN)

$(BIN): $(SOURCES)
	$(CC) $(SOURCES) -D_VERSION=\"$(VERSION)\" $(CFLAGS) $(LDFLAGS) -o $(BIN)

install:
	$(INSTALL) -D -m 0755 $(BIN) $(DESTDIR)/lib/cryptsetup/naskpass
	$(INSTALL) -D -m 0755 scripts/naskpass.inithook $(DESTDIR)/usr/share/initramfs-tools/hooks/naskpass
	$(INSTALL) -D -m 0755 scripts/naskpass.initscript $(DESTDIR)/usr/share/naskpass/naskpass.script.initramfs

uninstall:
	rm -f $(DESTDIR)/lib/cryptsetup/naskpass
	rm -f $(DESTDIR)/usr/share/initramfs-tools/hooks/naskpass
	rm -f $(DESTDIR)/usr/share/naskpass/naskpass.script.initramfs
	rmdir --ignore-fail-on-non-empty $(DESTDIR)/usr/share/naskpass

clean:
	rm -f $(BIN)

source:
	-dh_make --createorig -p naskpass_$(VERSION) -s -y

deb-clean:
	-rm ../naskpass_$(VERSION)*

deb: deb-clean source
	dpkg-buildpackage -tc -us -uc

.PHONY: all install clean
