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

  memset(buf, '\0', bufsiz);
  memset(buf, '\0', bufsiz);
  if (argc > 1)
    strncpy(buf, argv[1], bufsiz-1);

  mq_unlink("/testmq");
  assert( (mq_test = mq_open( "/testmq", O_CREAT | O_EXCL | O_RDWR  | O_NONBLOCK, S_IRUSR | S_IWUSR, NULL )) != (mqd_t)-1 );
  assert( mq_getattr(mq_test, &m_attr) == 0 );
  printf("flags.....: %ld\n"
         "maxmsg....: %ld\n"
         "msgsize...: %ld\n"
         "curmsg....: %ld\n",
         m_attr.mq_flags, m_attr.mq_maxmsg, m_attr.mq_msgsize, m_attr.mq_curmsgs);

  m_attr.mq_msgsize = bufsiz-1;
  assert ( mq_setattr(mq_test, &m_attr, NULL) == 0 );
  assert ( mq_send(mq_test, buf, bufsiz-1, 0) == 0 );
  assert ( mq_getattr(mq_test, &m_attr) == 0 );
  printf("new msgsize...: %ld\n"
         "new curmsg....: %ld\n",
         m_attr.mq_msgsize, m_attr.mq_curmsgs);
  assert ( mq_receive(mq_test, recv, bufsiz-1, 0) > 0 );

  printf("RECV: %s\n", recv);

  return 0;
}

