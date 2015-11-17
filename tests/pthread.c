#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include <assert.h>


static pthread_mutex_t testMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t testCond = PTHREAD_COND_INITIALIZER;

static void
mywait(int time_secs)
{
  struct timespec now;
  int rt;

  clock_gettime(CLOCK_REALTIME, &now);
  now.tv_sec += time_secs;

  assert( pthread_mutex_lock(&testMutex) == 0 );
  rt = pthread_cond_timedwait(&testCond, &testMutex, &now);
  assert( rt == ETIMEDOUT || rt == 0 );
  assert( pthread_mutex_unlock(&testMutex) == 0 );
  printf("Done.\n");
}

static void *
fun(void *arg)
{
  printf("Thread wait ..\n");
  mywait(1);
  mywait(1);
  return NULL;
}

int
main(void)
{
  pthread_t thread;
  void *ret;

  assert( pthread_create(&thread, NULL, fun, NULL) == 0 );
  usleep(50000);
  assert( pthread_cond_signal(&testCond) == 0 );
  printf("Mainthread: join\n");
  assert( pthread_join(thread, &ret) == 0 );
  return 0;
}
