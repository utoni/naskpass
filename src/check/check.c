#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <mqueue.h>

#define myassert(x) if ((x)) { ret &= 0xFF; } else { ret &= 0x00; }

static volatile unsigned char ret = 0xFF;
static mqd_t mq_test;
static mqd_t mq_recv;
static const size_t bufsiz = 256;

int main(int argc, char **argv)
{
  int c_stat;
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
  myassert( (mq_test = mq_open( "/testmq", O_NONBLOCK | O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG, &m_attr )) != (mqd_t)-1 );
  myassert( mq_getattr(mq_test, &m_attr) == 0 );

  strcpy(buf, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVQXYZ");
  myassert( mq_send(mq_test, buf, bufsiz, 0) == 0 );
  myassert( (sz_recv = mq_receive(mq_test, recv, bufsiz, &prio)) > 0 );

  memset(recv, '\0', bufsiz);
  if (fork() > 0) {
    myassert( (mq_recv = mq_open( "/testmq", O_RDONLY, S_IRWXU | S_IRWXG, &m_attr )) != (mqd_t)-1 );
    myassert( (sz_recv = mq_receive(mq_recv, recv, bufsiz, &prio)) > 0 );
    return ret;
  }
  myassert( mq_send(mq_test, buf, bufsiz, 0) == 0 );
  wait(&c_stat);
  myassert( c_stat == 0xFF );
  myassert( mq_close(mq_test) == 0 );
  myassert( mq_unlink("/testmq") == 0 );

  return ~ret;
}
