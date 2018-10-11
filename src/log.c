#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <syslog.h>

#include "log.h"

#define LOG_BUFSIZ 128

static FILE* logfile = NULL;


int log_init(char* file)
{
  if (!file) {
    openlog("naskpass", LOG_NDELAY | LOG_PID, LOG_DAEMON);
    return 0;
  } else {
    logfile = fopen(file, "a+");
    return (logfile ? 0 : errno);
  }
}

void log_free(void)
{
  if (!logfile) {
    closelog();
  } else {
    fclose(logfile);
    logfile = NULL;
  }
}

int logs(const char* format, ...)
{
  int ret;
  va_list vargs;

  va_start(vargs, format);
  if (!logfile) {
    vsyslog(LOG_DEBUG, format, vargs);
    ret = 0;
  } else {
    ret = vfprintf(logfile, format, vargs);
    fflush(logfile);
  }
  va_end(vargs);
  return ret;
}

