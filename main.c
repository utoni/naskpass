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

#include "config.h"

#include "ui.h"
#include "ui_ipc.h"
#include "ui_ani.h"
#include "ui_input.h"
#include "ui_statusbar.h"

#define MSG(msg_idx) msg_arr[msg_idx]


enum msg_index {
  MSG_BUSY_FD = 0,
  MSG_BUSY,
  MSG_NO_FIFO,
  MSG_FIFO_ERR,
  MSG_NUM
};
static const char *msg_arr[] = { "Please wait, got a piped password ..", "Please wait, busy ..",
                                 "check_fifo: %s is not a FIFO\n", "check_fifo: %s error(%d): %s\n" };


static void
usage(char *arg0)
{
  fprintf(stderr, "\n%s (%s)\n  %s\n", PKGNAME, VERSION, PKGDESC);
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
          fprintf(stderr, MSG(MSG_NO_FIFO), fifo_path);
          return (false);
        }
      }
    }
  }
  fprintf(stderr, MSG(MSG_FIFO_ERR), fifo_path, errno, strerror(errno));
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
  free(cmd);
  return (retval);
}

void sigfunc(int signal)
{
  switch (signal) {
    case SIGTERM:
    case SIGINT:
      ui_ipc_semtrywait(SEM_UI);
      ui_ipc_semtrywait(SEM_IN);
      break;
  }
}

int
main(int argc, char **argv)
{
  int ret = EXIT_FAILURE, ffd = -1, c_status, opt;
  pid_t child;
  char pbuf[IPC_MQSIZ+1];
  char *fifo_path = NULL;
  char *crypt_cmd = NULL;
  struct timespec ts_sem_input;

  signal(SIGINT, sigfunc);
  signal(SIGTERM, sigfunc);

  if ( clock_gettime(CLOCK_REALTIME, &ts_sem_input) == -1 ) {
    fprintf(stderr, "%s: clock get time error: %d (%s)\n", argv[0], errno, strerror(errno));
    goto error;
  }

  if (ui_ipc_init(1) != 0) {
    fprintf(stderr, "%s: can not create semaphore/message queue: %d (%s)\n", argv[0], errno, strerror(errno));
    goto error;
  }

  memset(pbuf, '\0', IPC_MQSIZ+1);
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

  ui_ipc_sempost(SEM_UI);
  if ((child = fork()) == 0) {
    /* child */
    fclose(stderr);
    /* Slave process: TUI */
    do_ui();
  } else if (child > 0) {
    /* parent */
    fclose(stdin);
    fclose(stdout);
    /* Master process: mainloop (read passwd from message queue or fifo and exec cryptcreate */
    while ( ui_ipc_getvalue(SEM_UI) > 0 || ui_ipc_getvalue(SEM_IN) > 0 ) {
      if (read(ffd, pbuf, IPC_MQSIZ) >= 0) {
        ui_ipc_sempost(SEM_BS);
        ui_ipc_msgsend(MQ_IF, MSG(MSG_BUSY_FD), strlen(MSG(MSG_BUSY_FD)));
        if (run_cryptcreate(pbuf, crypt_cmd) != 0) {
          fprintf(stderr, "cryptcreate error\n");
        }
      } else if ( ui_ipc_msgrecv(MQ_PW, pbuf, IPC_MQSIZ) > 0 ) {
        ui_ipc_sempost(SEM_BS);
        ui_ipc_msgsend(MQ_IF, MSG(MSG_BUSY), strlen(MSG(MSG_BUSY)));
        if (run_cryptcreate(pbuf, crypt_cmd) != 0) {
          fprintf(stderr, "cryptcreate error\n");
        }
        ui_ipc_semwait(SEM_IN);
      }
      usleep(100000);
    }
    wait(&c_status);
    memset(pbuf, '\0', IPC_MQSIZ+1);
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
  ui_ipc_free(1);
  exit(ret);
}
