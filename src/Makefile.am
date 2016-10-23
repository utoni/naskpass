bin_PROGRAMS=naskpass naskshell naskpass_check
naskpass_SOURCES=main.c log.c opt.c utils.c status.c ui_ani.c ui.c ui_input.c ui_ipc.c ui_txtwindow.c ui_statusbar.c ui_nask.c
naskpass_LDFLAGS=
naskpass_check_SOURCES=check/check.c
naskshell_SOURCES=shell/shell.c
if DEBUG
naskpass_CFLAGS=-O0 -g3 -DDEBUG
else
naskpass_CFLAGS=-fPIC -fomit-frame-pointer -fexpensive-optimizations -Os
endif
if RESOLVE
naskpass_CFLAGS+=-DHAVE_RESOLVE
endif
if UNAME
naskpass_CFLAGS+=-DHAVE_UNAME
endif
