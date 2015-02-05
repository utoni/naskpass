#define AUTHOR "Toni Uhlig"
#define AUTHOR_EMAIL "matzeton@googlemail.com"
#define PKGNAME "naskpass"
#define PKGDESC "A NCurses replacement for cryptsetup's askpass."
#define DEFAULT_FIFO "/lib/cryptsetup/passfifo"
#define SHTDWN_CMD "echo 'o' >/proc/sysrq-trigger"

#ifdef _VERSION
#define VERSION _VERSION
#else
#define VERSION "unknown"
#endif
