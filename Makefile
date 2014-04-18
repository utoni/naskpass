CFLAGS ?= $(shell ncurses5-config --cflags) -Wall -D_GNU_SOURCE
LDFLAGS ?= $(shell ncurses5-config --libs) -pthread
CC ?= gcc
INSTALL ?= install
VERSION = 1.0
BIN = nask

all: bin

bin:
	$(CC) main.c $(CFLAGS) $(LDFLAGS) -o $(BIN)

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
