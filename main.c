#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>

#include "ui_ani.h"
#include "ui_input.h"
#include "ui_statusbar.h"
#include "ui.h"

#define PKGNAME "nask"
#ifdef _VERSION
#define VERSION _VERSION
#else
#define VERSION "unknown"
#endif
#define SHTDWN_CMD "echo 'o' >/proc/sysrq-trigger"



static void usage(char *arg0)
{
  fprintf(stderr, "%s (%s)\n", PKGNAME, VERSION);
  fprintf(stderr, "  CMD: %s: [passfifo]\n", arg0);
}

int main(int argc, char **argv)
{
  pid_t child;
  int status;

  if (argc != 2) {
    usage(argv[0]);
    exit(-1);
  }

  if ((child = vfork()) == 0) {
    if (setsid() == (pid_t)-1) {
      perror("setsid");
      return (EXIT_FAILURE);
    }
    do_ui();
  } if (child > 0) {
    wait(&status);
  }else {
    return (EXIT_FAILURE);
  }
  return (EXIT_SUCCESS);
}
