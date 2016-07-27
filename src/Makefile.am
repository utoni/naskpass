bin_PROGRAMS=naskpass naskshell naskpass_check
naskpass_SOURCES=main.c log.c opt.c status.c ui_ani.c ui.c ui_input.c ui_ipc.c ui_txtwindow.c ui_statusbar.c ui_nask.c
naskpass_check_SOURCES=check/check.c
naskshell_SOURCES=shell/shell.c
if DEBUG
naskpass_CFLAGS=-O0 -g3 -DDEBUG
else
naskpass_CFLAGS=-fPIC -fomit-frame-pointer -Os
endif
