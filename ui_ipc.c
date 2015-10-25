#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#ifdef SEM_TIMEDWAIT
#include <time.h>
#endif
#include <semaphore.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <errno.h>

#include "ui_ipc.h"

#define JMP_IF(cmd, retval, jmplabel) if ( (cmd) == retval ) {  printf("(%s) == %p\n", #cmd, (void*)retval);  goto jmplabel; }


static sem_t *sems[SEM_NUM];
static mqd_t msqs[MSQ_NUM];
static unsigned char initialized = 0;


int
ui_ipc_init(int is_master)
{
  volatile int sp_oflags, mq_oflags;
  mode_t crt_flags;
  struct mq_attr m_attr;

  if (initialized) {
    return -1;
  }
  bzero(sems, sizeof(sem_t*)*SEM_NUM);
  bzero(msqs, sizeof(mqd_t)*MSQ_NUM);
  m_attr.mq_flags = 0;
  m_attr.mq_msgsize = IPC_MQSIZ;
  m_attr.mq_maxmsg = 3;
  m_attr.mq_curmsgs = 0;
  if (is_master) {
    sp_oflags = O_CREAT | O_EXCL;
    mq_oflags = O_NONBLOCK | O_CREAT | O_EXCL;
    crt_flags = S_IRUSR | S_IWUSR;
    JMP_IF( msqs[MQ_PW]  = mq_open(MSQ_PWD, mq_oflags | O_RDONLY, crt_flags, &m_attr), (mqd_t)-1, error );
    JMP_IF( msqs[MQ_IF]  = mq_open(MSQ_INF, mq_oflags | O_WRONLY, crt_flags, &m_attr), (mqd_t)-1, error );
  } else {
    sp_oflags = 0;
    mq_oflags = 0;
    crt_flags = 0;
    JMP_IF( msqs[MQ_PW]  = mq_open(MSQ_PWD, mq_oflags | O_WRONLY, crt_flags, &m_attr), (mqd_t)-1, error );
    JMP_IF( msqs[MQ_IF]  = mq_open(MSQ_INF, mq_oflags | O_RDONLY, crt_flags, &m_attr), (mqd_t)-1, error );
  }
  JMP_IF( sems[SEM_UI] = sem_open(SEM_GUI, sp_oflags, crt_flags, 0), SEM_FAILED, error );
  JMP_IF( sems[SEM_IN] = sem_open(SEM_INP, sp_oflags, crt_flags, 0), SEM_FAILED, error );
  JMP_IF( sems[SEM_BS] = sem_open(SEM_BSY, sp_oflags, crt_flags, 0), SEM_FAILED, error );
  JMP_IF( sems[SEM_RD] = sem_open(SEM_RDY, sp_oflags, crt_flags, 0), SEM_FAILED, error );
  initialized = 1;
  return 0;
error:
  return errno;
}

void
ui_ipc_free(int is_master)
{
  int i;

  if (!initialized) {
    return;
  }
  for (i = 0; i < SEM_NUM; i++) {
    if (sems[i]) sem_close(sems[i]);
  }
  for (i = 0; i < MSQ_NUM; i++) {
    if (msqs[i]) mq_close(msqs[i]);
  }
  if (is_master > 0) {
    sem_unlink(SEM_BSY);
    sem_unlink(SEM_GUI);
    sem_unlink(SEM_INP);
    sem_unlink(SEM_RDY);
    mq_unlink(MSQ_PWD);
    mq_unlink(MSQ_INF);
  }
  initialized = 0;
}

int
ui_ipc_sempost(enum UI_IPC_SEM e_sp)
{
  return ( sem_post(sems[e_sp]) );
}

int
ui_ipc_semwait(enum UI_IPC_SEM e_sp)
{
  return ( sem_wait(sems[e_sp]) );
}

int
ui_ipc_semtrywait(enum UI_IPC_SEM e_sp)
{
  return ( sem_trywait(sems[e_sp]) );
}

int
ui_ipc_getvalue(enum UI_IPC_SEM e_sp)
{
  int sp_val = 0;

  if (sem_getvalue(sems[e_sp], &sp_val) != 0) {
    return -1;
  }
  return sp_val;
}

#ifdef SEM_TIMEDWAIT
int
ui_ipc_semtimedwait(enum UI_IPC_SEM e_sp, int timeout)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
    return -1;
  }
  ts.tc_sec += timeout;
  return ( sem_timedwait(sems[q_mq], &ts) );
}
#endif

int
ui_ipc_msgsend(enum UI_IPC_MSQ e_mq, const char *msg_ptr, size_t msg_len)
{
  return ( mq_send(msqs[e_mq], msg_ptr, msg_len, 0) );
}

ssize_t
ui_ipc_msgrecv(enum UI_IPC_MSQ e_mq, char *msg_ptr, size_t msg_len)
{
  return ( mq_receive(msqs[e_mq], msg_ptr, msg_len, NULL) ); 
}

