#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>

#include "log.h"

#define LOG_BUFSIZ 128

static FILE* logfile = NULL;


int log_init(char* file)
{
  if (!file) return -1;
  logfile = fopen(file, "a+");
  return (logfile ? 0 : errno);
}

void log_free(void)
{
  if (logfile)
    fclose(logfile);
  logfile = NULL;
}

int logs(char* format, ...)
{
  int ret;
  va_list vargs;

  if (!logfile) return -1;
  va_start(vargs, format);
  ret = vfprintf(logfile, format, vargs);
  fflush(logfile);
  va_end(vargs);
  return ret;
}

