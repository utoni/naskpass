#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include <assert.h>


static mqd_t mq_test;
static const size_t bufsiz = 256;

int main(int argc, char **argv)
{
  struct mq_attr m_attr;
  char buf[bufsiz], recv[bufsiz];
  unsigned int prio;
  ssize_t sz_recv;

  memset(buf, '\0', bufsiz);
  memset(recv, '\0', bufsiz);
  if (argc > 1)
    strncpy(buf, argv[1], bufsiz-1);

  m_attr.mq_flags = 0;
  m_attr.mq_msgsize = bufsiz;
  m_attr.mq_maxmsg = 10;
  m_attr.mq_curmsgs = 0;

  mq_unlink("/testmq");
  assert( (mq_test = mq_open( "/testmq", O_NONBLOCK | O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG, &m_attr )) != (mqd_t)-1 );
  assert( mq_getattr(mq_test, &m_attr) == 0 );
  printf("flags.........: %ld\n"
         "maxmsg........: %ld\n"
         "msgsize.......: %ld\n"
         "curmsg........: %ld\n",
         m_attr.mq_flags, m_attr.mq_maxmsg, m_attr.mq_msgsize, m_attr.mq_curmsgs);

  strcpy(buf, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVQXYZ");
  assert ( mq_send(mq_test, buf, bufsiz, 0) == 0 );
  assert ( (sz_recv = mq_receive(mq_test, recv, bufsiz, &prio)) > 0 );

  printf("SENT(%03lu bytes): %s\n", (long unsigned int) strlen(buf), buf);
  printf("RECV(%03lu bytes): %s\n", (long unsigned int) sz_recv, recv);

  return 0;
}

