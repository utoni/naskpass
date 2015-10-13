#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


sem_t *mysem = NULL;
pid_t child;

#define LOG(cmd) fprintf(stderr, "%s\n", cmd);

int main(int argc, char **argv) {
  sem_unlink("/mysem");
  if ( (mysem = sem_open("/mysem", O_CREAT, S_IRUSR | S_IWUSR, 1)) != NULL ) {
    if ( (child = fork()) == 0 ) {
      /* child */
      sleep(1);
      LOG("child: sempost");
      sem_post(mysem);
      LOG("child: done");
      sleep(1);
      exit(0);
    } else if (child > 0) {
      /* parent */
      LOG("parent: semwait");
      sem_wait(mysem);
      LOG("parent: waitpid");
      waitpid(child, NULL, 0);
    } else if (child < 0) {
      perror("fork");
    }
  } else {
    sem_close(mysem);
    exit(1);
  }
  exit(0);
}

