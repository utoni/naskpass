#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "ui_ani.h"
#include "ui_input.h"
#include "ui_statusbar.h"
#include "ui.h"
#include "config.h"


static bool ui_active = true;

static void
usage(char *arg0)
{
  fprintf(stderr, "%s (%s)\n  %s\n", PKGNAME, VERSION, PKGDESC);
  fprintf(stderr, "  Written by %s (%s).\n", AUTHOR, AUTHOR_EMAIL);
  fprintf(stderr, "  License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n\n");
  fprintf(stderr, "  Command:\n\t%s [args]\n", arg0);
  fprintf(stderr, "  Arguments:\n\t-h this\n\t-f [passfifo] default: %s\n\t-c [cryptcreate]\n", DEFAULT_FIFO);
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

/* stolen from http://www.gnu.org/software/libc/manual/html_node/Waiting-for-I_002fO.html */
static int
input_timeout(int filedes, unsigned int seconds)
{
  fd_set set;
  struct timeval timeout;

  /* Initialize the file descriptor set. */
  FD_ZERO (&set);
  FD_SET (filedes, &set);
  /* Initialize the timeout data structure. */
  timeout.tv_sec = seconds;
  timeout.tv_usec = 0;
  /* select returns 0 if timeout, 1 if input available, -1 if error. */
  return TEMP_FAILURE_RETRY(select(FD_SETSIZE, &set, NULL, NULL, &timeout));
}

int
run_cryptcreate(char *pass, char *crypt_cmd)
{
  int retval;
  char *cmd;

  if (crypt_cmd == NULL || pass == NULL) return (-1);
  asprintf(&cmd, "echo '%s' | %s", pass, crypt_cmd);
  retval = system(cmd);
  return (retval);
}

int
main(int argc, char **argv)
{
  int ffd, c_status, opt;
  pid_t child;
  char pbuf[MAX_PASSWD_LEN+1];
  char *fifo_path = NULL;
  char *crypt_cmd = NULL;

  memset(pbuf, '\0', MAX_PASSWD_LEN+1);

  while ((opt = getopt(argc, argv, "hf:c:")) != -1) {
    switch (opt) {
      case 'h':
        usage(argv[0]);
        exit(EXIT_SUCCESS);
      case 'f':
        fifo_path = strdup(optarg);
        break;
      case 'c':
        crypt_cmd = strdup(optarg);
        break;
      default:
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
  }
  if (optind < argc) {
    fprintf(stderr, "%s: I dont understand you.\n\n", argv[0]);
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }
  if (fifo_path == NULL) fifo_path = strdup(DEFAULT_FIFO);

  if (check_fifo(fifo_path) == false) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }
  if ((ffd = open(fifo_path, O_NONBLOCK | O_RDWR)) < 0) {
    fprintf(stderr, "fifo: %s\n", fifo_path);
    perror("open");
    exit(EXIT_FAILURE);
  }

  if ((child = fork()) == 0) {
    /* child */
    ui_active = true;
    do_ui(ffd);
    ui_active = false;
  } else if (child > 0) {
    /* parent */
    fclose(stdin);
    while (input_timeout(ffd, 1) == 0) {
      usleep(100000);
      if (ui_active == true) {
        
      }
    }
    stop_ui();
    wait(&c_status);
    if (read(ffd, pbuf, MAX_PASSWD_LEN) > 0) {
      if (run_cryptcreate(pbuf, crypt_cmd) != 0) {
        fprintf(stderr, "cryptcreate error\n");
      }
    }
    memset(pbuf, '\0', MAX_PASSWD_LEN+1);
  } else {
    /* fork error */
    perror("fork");
    exit(EXIT_FAILURE);
  }

printf("BLA\n");
  close(ffd);
  if (crypt_cmd != NULL) free(crypt_cmd);
  free(fifo_path);
  return (EXIT_SUCCESS);
}
