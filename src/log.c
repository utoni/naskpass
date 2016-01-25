#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>

#include "log.h"

#define LOG_BUFSIZ 128

static FILE* logfile;


int log_init(char* file)
{
  logfile = fopen(file, "a+");
  return (logfile ? 0 : errno);
}

int logs(char* format, ...)
{
  int ret;
  char* buf;
  va_list vargs;

  buf = (char*) calloc(LOG_BUFSIZ, sizeof(char));
  va_start(vargs, format);
  ret = vsnprintf(buf, LOG_BUFSIZ, format, vargs);
  va_end(vargs);
  return ret;
}

