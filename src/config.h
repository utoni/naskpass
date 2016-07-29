#define AUTHOR "Toni Uhlig"
#define AUTHOR_EMAIL "matzeton@googlemail.com"
#define PKGNAME "naskpass"
#define PKGDESC "A NCurses replacement for cryptsetup's askpass."
#define DEFAULT_FIFO "/lib/cryptsetup/passfifo"
#define SHTDWN_CMD "echo 'o' >/proc/sysrq-trigger"

#define SEM_GUI "/naskpass-gui"
#define SEM_INP "/naskpass-input"
#define MSQ_PWD "/naskpass-passwd"
#define MSQ_INF "/naskpass-info"

#ifdef HAVE_CONFIG_H
#include "version.h"
#endif

#ifndef VERSION
#define VERSION "unknown"
#endif
