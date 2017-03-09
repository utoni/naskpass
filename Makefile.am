SUBDIRS = src

install-exec-local:
	install -d -m755 '$(prefix)/lib/cryptsetup'
	install -m755 -c src/naskpass '$(prefix)/lib/cryptsetup/naskpass'
	install -m755 -c src/naskshell '$(prefix)/lib/cryptsetup/naskshell'
	install -m755 -c src/naskpass_check '$(prefix)/lib/cryptsetup/naskpass_check'
	rm '$(prefix)/bin/naskpass'
	rm '$(prefix)/bin/naskshell'
	rm '$(prefix)/bin/naskpass_check'
	rmdir '$(prefix)/bin'

clean-local:
	rm -f naskpass naskshell naskpass_check
	$(MAKE) -C ./tests clean

distclean-local: clean-local
	rm -f config.log config.status
	rm -f src/aconfig.h.in~ src/*.d src/version.h
	rm -rf autom4te.cache

