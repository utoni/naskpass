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
#include <semaphore.h>
#include <time.h>
#include <mqueue.h>

#include "ui_ani.h"
#include "ui_input.h"
#include "ui_statusbar.h"
#include "ui.h"
#include "config.h"


static sem_t *sp_ui, *sp_input;
static mqd_t mq_passwd;


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

void sigfunc(int signal)
{
  switch (signal) {
    case SIGTERM:
    case SIGKILL:
    case SIGINT:
      sem_trywait(sp_ui);
      sem_trywait(sp_input);
      break;
  }
}

int
main(int argc, char **argv)
{
  int ret = EXIT_FAILURE, ffd = -1, c_status, opt, i_sval;
  pid_t child;
  char pbuf[MAX_PASSWD_LEN+1];
  char *fifo_path = NULL;
  char *crypt_cmd = NULL;
  struct timespec ts_sem_input;
  struct mq_attr mq_attr;

  signal(SIGINT, sigfunc);
  signal(SIGTERM, sigfunc);
  signal(SIGKILL, sigfunc);

  if ( clock_gettime(CLOCK_REALTIME, &ts_sem_input) == -1 ) {
    fprintf(stderr, "%s: clock get time error: %d (%s)\n", argv[0], errno, strerror(errno));
    goto error;
  }
  if ( (sp_ui = sem_open(SEM_GUI, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED ||
          (sp_input = sem_open(SEM_INP, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED ||
(mq_passwd = mq_open(MSQ_PWD, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, NULL)) == (mqd_t) -1 ) {
    if ( errno == EEXIST ) {
      fprintf(stderr, "%s: already started?\n", argv[0]);
    } else {
      fprintf(stderr, "%s: can not create semaphore: %d (%s)\n", argv[0], errno, strerror(errno));
    }
    goto error;
  }
  if ( mq_getattr(mq_passwd, &mq_attr) == 0 ) {
    mq_attr.mq_maxmsg = 2;
    mq_attr.mq_msgsize = MAX_PASSWD_LEN;
    if ( mq_setattr(mq_passwd, &mq_attr, NULL) != 0 ) {
      fprintf(stderr, "%s: can not SET message queue attributes: %d (%s)\n", argv[0], errno, strerror(errno));
      goto error;
    }
  } else {
    fprintf(stderr, "%s: can not GET message queue attributes: %d (%s)\n", argv[0], errno, strerror(errno));
    goto error;
  }

  memset(pbuf, '\0', MAX_PASSWD_LEN+1);
  while ((opt = getopt(argc, argv, "hf:c:")) != -1) {
    switch (opt) {
      case 'h':
        usage(argv[0]);
        goto error;
      case 'f':
        fifo_path = strdup(optarg);
        break;
      case 'c':
        crypt_cmd = strdup(optarg);
        break;
      default:
        usage(argv[0]);
        goto error;
    }
  }
  if (optind < argc) {
    fprintf(stderr, "%s: I dont understand you.\n\n", argv[0]);
    usage(argv[0]);
    goto error;
  }
  if (fifo_path == NULL) fifo_path = strdup(DEFAULT_FIFO);

  if (check_fifo(fifo_path) == false) {
    usage(argv[0]);
    goto error;
  }
  if ((ffd = open(fifo_path, O_NONBLOCK | O_RDWR)) < 0) {
    fprintf(stderr, "%s: fifo '%s' error: %d (%s)\n", argv[0], fifo_path, errno, strerror(errno));
    goto error;
  }

  if ((child = fork()) == 0) {
    /* child */
    fclose(stderr);
    /* Slave process: TUI */
    sem_post(sp_ui);
    do_ui();
  } else if (child > 0) {
    /* parent */
    fclose(stdin);
    fclose(stdout);
    /* Master process: mainloop (read passwd from message queue or fifo and exec cryptcreate */
    while ( sem_getvalue(sp_ui, &i_sval) == 0 && i_sval > 0 ) {
      if ( sem_getvalue(sp_input, &i_sval) == 0 && i_sval > 0 ) {
        if (read(ffd, pbuf, MAX_PASSWD_LEN) > 0) {
          if (run_cryptcreate(pbuf, crypt_cmd) != 0) {
            fprintf(stderr, "cryptcreate error\n");
          }
        }
      } else if ( mq_receive(mq_passwd, pbuf, MAX_PASSWD_LEN, NULL) > 0 ) {
exit(77);
      }
      usleep(100000);
    }
    wait(&c_status);
    memset(pbuf, '\0', MAX_PASSWD_LEN+1);
  } else {
    /* fork error */
    perror("fork");
    goto error;
  }

  ret = EXIT_SUCCESS;
error:
  if (ffd >= 0) close(ffd);
  if (crypt_cmd != NULL) free(crypt_cmd);
  if (fifo_path != NULL) free(fifo_path);
  sem_close(sp_ui);
  sem_close(sp_input);
  mq_close(mq_passwd);
  sem_unlink(SEM_GUI);
  sem_unlink(SEM_INP);
  mq_unlink(MSQ_PWD);
  exit(ret);
}
