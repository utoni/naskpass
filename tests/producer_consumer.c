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

sem_t *mysem = NULL, *mycnt = NULL;
pid_t child;


int main(int argc, char **argv) {
  int semval = 0;

  if ( (mysem = sem_open(TESTSEM, 0, S_IRUSR | S_IWUSR, 1)) != NULL &&
          (mycnt = sem_open(CNTSEM, 0, S_IRUSR | S_IWUSR, 1)) != NULL ) {
    assert( sem_getvalue(mycnt, &semval) == 0 );
    printf("factory producing %d items\n", semval);

    while ( semval-- >= 0 ) {
      usleep(250);
      LOG("consumer: -1");
      assert( sem_wait(mysem) == 0 );
      printf("remaining: %d\n", semval);
    }
  } else {
    exit(1);
  }
  assert( sem_close(mysem) == 0 );
  assert( sem_unlink(TESTSEM) == 0 && sem_unlink(CNTSEM) == 0 );

  exit(0);
}

