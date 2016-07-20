#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <mqueue.h>


#define myassert(x, emask) if ((x)) { ret &= ret; } else { ret |= emask; }

static volatile unsigned long long int ret = 0x0;
static mqd_t mq_test;
static mqd_t mq_recv;
static sem_t *sp_test;
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
  mq_test = mq_open( "/testmq", O_NONBLOCK | O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG, &m_attr );
  myassert( mq_test != (mqd_t)-1, 0x1 );
  myassert( mq_getattr(mq_test, &m_attr) == 0, 0x2 );

  strcpy(buf, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVQXYZ");
  myassert( mq_send(mq_test, buf, bufsiz, 0) == 0, 0x4 );
  sz_recv = mq_receive(mq_test, recv, bufsiz, &prio);
  myassert( sz_recv > 0, 0x8 );

  memset(recv, '\0', bufsiz);
  if (fork() > 0) {
    mq_recv = mq_open( "/testmq", O_RDONLY, S_IRWXU | S_IRWXG, &m_attr );
    myassert( mq_recv != (mqd_t)-1, 0x10 );
    sz_recv = mq_receive(mq_recv, recv, bufsiz, &prio);
    myassert( sz_recv > 0, 0x20 );
    return ret;
  }
  myassert( mq_send(mq_test, buf, bufsiz, 0) == 0, 0x40 );
  wait(&c_stat);
  myassert( c_stat == 0xFF, 0x80 );
  myassert( mq_close(mq_test) == 0, 0x100 );
  myassert( mq_unlink("/testmq") == 0, 0x200 );

  myassert( sem_unlink("/testsem") == 0, 0x400 );
  sp_test = sem_open("/testsem", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);
  myassert( sp_test == 0, 0x800 );
  myassert( sem_post(sp_test) == 0, 0x1000 );
  myassert( sem_wait(sp_test) == 0, 0x1200 );
  myassert( sem_close(sp_test) == 0, 0x1400 );
  myassert( sem_unlink("/testsem") == 0, 0x1800 );

  return ret;
}
