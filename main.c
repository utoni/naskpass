#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ui_ani.h"
#include "ui_input.h"
#include "ui_statusbar.h"
#include "ui.h"

#define AUTHOR "Toni Uhlig"
#define AUTHOR_EMAIL "matzeton@googlemail.com"
#define PKGNAME "naskpass"
#define PKGDESC "A NCurses replacement for cryptsetup's askpass."
#ifdef _VERSION
#define VERSION _VERSION
#else
#define VERSION "unknown"
#endif

#define SHTDWN_CMD "echo 'o' >/proc/sysrq-trigger"



static void
usage(char *arg0)
{
  fprintf(stderr, "%s (%s)\n  %s\n", PKGNAME, VERSION, PKGDESC);
  fprintf(stderr, "  Written by %s (%s).\n", AUTHOR, AUTHOR_EMAIL);
  fprintf(stderr, "  License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n\n");
  fprintf(stderr, "  Command: %s: [passfifo]\n", arg0);
}

static bool
check_fifo(char *fifo_path)
{
  struct stat st;

  if (mkfifo(fifo_path, S_IRUSR | S_IWUSR) == 0) {
    return (true);
  } else {
    if (errno == EEXIST) {
      if (stat(fifo_path, &st) == 0) {
        if (S_ISFIFO(st.st_mode) == 1) {
          return (true);
        } else {
          fprintf(stderr, "stat: %s is not a FIFO\n", fifo_path);
          return (false);
        }
      }
    }
  }
  perror("check_fifo");
  return (false);
}

int main(int argc, char **argv)
{
  int ffd;
  size_t plen;

  if (argc != 2) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  if (check_fifo(argv[1]) == false) {
    exit(EXIT_FAILURE);
  }
  if ((ffd = open(argv[1], O_NONBLOCK | O_RDWR)) < 0) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  do_ui();
  if (passwd != NULL) {
    plen = strlen(passwd);
    printf("Sending your password to the FIFO ..\n");
    if (write(ffd, (const void *)passwd, plen) != plen) {
     perror("write");
    } else {
      printf("Ok.\n");
    }
    memset(passwd, '\0', plen);
    plen = 0;
    free(passwd);
  }

  close(ffd);
  return (EXIT_SUCCESS);
}
