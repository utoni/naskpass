#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "../ui_ipc.h"

pid_t child;

#define LOG(cmd) fprintf(stderr, "%s\n", cmd);

int main(int argc, char **argv) {
  if (ui_ipc_init(1) != 0) {
    perror("ui_ipc_init");
    exit(1);
  }
  if ( (child = fork()) == 0 ) {
    /* child */
    sleep(1);
    LOG("child: sempost");
    ui_ipc_sempost(SEM_RD);
    LOG("child: done");
    sleep(1);
    exit(0);
  } else if (child > 0) {
    /* parent */
    LOG("parent: semwait");
    ui_ipc_semwait(SEM_RD);
    LOG("parent: waitpid");
    waitpid(child, NULL, 0);
  } else if (child < 0) {
    perror("fork");
    exit(1);
  }
  ui_ipc_free(1);

  exit(0);
}

