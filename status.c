#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/sysinfo.h>

#include "status.h"


char *
get_system_stat(void)
{
  char *retstr = NULL;
  int ncpu;
  struct sysinfo inf;

  if (sysinfo(&inf) == EFAULT) {
    return ("[SYSINFO ERROR]");
  }
  ncpu = get_nprocs();

  if (asprintf(&retstr, "u:%04ld - l:%3.2f,%3.2f,%3.2f - %dcore%s - mem:%lu/%lumb - procs:%02d",
    inf.uptime, ((float)inf.loads[0]/10000), ((float)inf.loads[1]/10000), ((float)inf.loads[2]/10000),
    ncpu, (ncpu > 1 ? "s" : ""),
    (unsigned long)((inf.freeram/1024)/1024), (unsigned long)((inf.totalram/1024)/1024), inf.procs) == -1) {
    return ("[ASPRINTF ERROR]");
  }
  return (retstr);
}
