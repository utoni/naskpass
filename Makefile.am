SUBDIRS = src

install-exec-local:
	install -d -m755 '$(prefix)/lib/cryptsetup'
	install -c src/naskpass '$(prefix)/lib/cryptsetup/naskpass'
	install -c src/naskshell '$(prefix)/lib/cryptsetup/naskshell'
	install -c src/naskpass_check '$(prefix)/lib/cryptsetup/naskpass_check'
	rm '$(prefix)/bin/naskpass'
	rm '$(prefix)/bin/naskshell'
	rm '$(prefix)/bin/naskpass_check'
	rmdir '$(prefix)/bin'

clean-local:
	rm -f naskpass naskshell naskpass_check

distclean-local: clean-local
	rm -f aclocal.m4 config.log config.status configure
	rm -f Makefile.in src/Makefile.in src/aconfig.h.in~ src/*.d src/version.h
	rm -rf build autom4te.cache

