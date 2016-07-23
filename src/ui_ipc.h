#ifndef UI_IPC_H
#define UI_IPC_H 1

#include "status.h"
#include "config.h"

#define IPC_MQSIZ 128


enum UI_IPC_SEM {
  SEM_UI = 0,  /* TUI active? */
  SEM_IN,      /* Textfield has input avail */
  SEM_BS,      /* Master process busy */
  SEM_NUM
};

enum UI_IPC_MSQ {
  MQ_PW = 0,
  MQ_IF,
  MSQ_NUM
};


int
ui_ipc_init(int is_master);

void
ui_ipc_free(int is_master);

int
ui_ipc_sempost(enum UI_IPC_SEM e_sp);

int
ui_ipc_semwait(enum UI_IPC_SEM e_sp);

int
ui_ipc_semtrywait(enum UI_IPC_SEM e_sp);

int
ui_ipc_getvalue(enum UI_IPC_SEM e_sp);

#ifdef SEM_TIMEDWAIT
int
ui_ipc_semtimedwait(enum UI_IPC_MSQ e_sp, int timeout);
#endif

int
ui_ipc_msgsend(enum UI_IPC_MSQ e_mq, const char *msg_ptr);

ssize_t
ui_ipc_msgrecv(enum UI_IPC_MSQ e_mq, char *msg_ptr);

long
ui_ipc_msgcount(enum UI_IPC_MSQ e_mq);

#endif
