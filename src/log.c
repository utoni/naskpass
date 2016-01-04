#include <stdio.h>
#include <stdlib.h>

#include "log.h"


static int logfd;

int log_init(char *file)
{
  logfd = fopen(file, "a+");
  return logfd;
}

int logs(char *format, ...)
{
  
}

