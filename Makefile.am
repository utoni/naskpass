SUBDIRS = src

install-exec-local:
	/bin/mkdir -p '$(prefix)/lib/cryptsetup'
	/usr/bin/install -c src/naskpass '$(prefix)/lib/cryptsetup/naskpass'
	/usr/bin/install -c src/naskpass_check '$(prefix)/lib/cryptsetup/naskpass_check'
	rm '$(prefix)/bin/naskpass'
	rm '$(prefix)/bin/naskpass_check'
	rmdir '$(prefix)/bin'

clean-local:
	rm -f naskpass naskpass_check

distclean-local: clean-local
	rm -f aclocal.m4 config.log config.status configure
	rm -f Makefile.in src/Makefile.in src/aconfig.h.in~ src/*.d
	rm -rf build autom4te.cache

