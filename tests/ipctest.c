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
#include "../src/ui_ipc.h"



int main(int argc, char **argv) {
  int ret, c_status;
  pid_t child;

  ret = ui_ipc_init(1);

  ret |= ui_ipc_sempost(SEM_UI);
  ret |= ui_ipc_sempost(SEM_UI);
  assert(ui_ipc_getvalue(SEM_UI) == 2);
  if ( (child = fork()) == 0 ) {
    printf("child: wait (%d)\n", ui_ipc_getvalue(SEM_UI));
    ret |= ui_ipc_semtrywait(SEM_UI);
    assert(ui_ipc_getvalue(SEM_UI) == 1);
    ret |= ui_ipc_semtrywait(SEM_UI);
    assert(ui_ipc_getvalue(SEM_UI) == 0);
    ret |= ui_ipc_semwait(SEM_UI);
    printf("child: done (%d)\n", ui_ipc_getvalue(SEM_UI));
    ret |= ui_ipc_sempost(SEM_UI);
    exit( (ret == 0 ? 0 : ret) );
  } else if (child > 0) {
    usleep(100000);
    printf("parent: post (%d)\n", ui_ipc_getvalue(SEM_UI));
    ui_ipc_sempost(SEM_UI);
  } else {
    ret |= 1;
  }

  wait(&c_status);
  ret |= ui_ipc_semtrywait(SEM_UI);
  ui_ipc_free(1);
  exit( c_status | ret );
}

