#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>

#include "semconfig.h"


sem_t *mysem = NULL;
pid_t child;


int main(int argc, char **argv) {
  sem_unlink(TESTSEM);
  if ( (mysem = sem_open(TESTSEM, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) != NULL ) {
    if ( (child = fork()) == 0 ) {
      /* child */
      usleep(250);
      LOG("child: sempost");
      sem_post(mysem);
      int tmp = 0;
      assert ( sem_getvalue(mysem, &tmp) == 0 && tmp == 1 );
      LOG("child: done");
      usleep(250);
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

  int sval = 1;
  assert ( sem_getvalue(mysem, &sval) == 0 && sval == 0 );
  assert (sval == 0);

  sem_unlink(TESTSEM);
  exit(0);
}

