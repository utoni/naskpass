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
  int semval = 10;

  if (argc == 2) {
    semval = atoi(argv[1]);
  }

  if ( (mysem = sem_open(TESTSEM, O_CREAT, S_IRUSR | S_IWUSR, 0)) != NULL &&
         (mycnt = sem_open(CNTSEM, O_CREAT, S_IRUSR | S_IWUSR, semval)) != NULL ) {
    while (semval-- >= 0) {
      sleep(1);
      LOG("producer: +1");
      assert( sem_post(mysem) == 0 );
      printf("remaining: %d\n", semval);
    }
    assert( sem_close(mysem) == 0 && sem_close(mycnt) == 0 );
    assert( sem_unlink(TESTSEM) == 0 && sem_unlink(CNTSEM) == 0 );
  } else {
    exit(1);
  }
  exit(0);
}

