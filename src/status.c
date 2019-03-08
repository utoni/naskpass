#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/sysinfo.h>

#include "status.h"

static const char *sysinfo_error = "[SYSINFO ERROR]";
static const char *asprintf_error = "[ASPRINTF ERROR]";


char *
get_system_stat(size_t *siz)
{
  char *retstr = NULL;
  int ncpu, retsiz;
  struct sysinfo inf;

  if (sysinfo(&inf) == EFAULT) {
    if (siz) {
      *siz = strlen(sysinfo_error);
    }
    return (strdup(sysinfo_error));
  }
  ncpu = get_nprocs();

  retsiz = asprintf(&retstr, "u:%04ld - l:%3.2f,%3.2f,%3.2f - %dcore%s - "
                             "mem:%lu/%lumb - procs:%02d",
                             inf.uptime, ((float)inf.loads[0]/10000),
                             ((float)inf.loads[1]/10000), ((float)inf.loads[2]/10000),
                             ncpu, (ncpu > 1 ? "s" : ""),
                             (unsigned long)((inf.freeram/1024)/1024),
                             (unsigned long)((inf.totalram/1024)/1024), inf.procs);

  if (retsiz < 0) {
    if (siz) {
      *siz = strlen(asprintf_error);
    }
    return (strdup(asprintf_error));
  }
  if (siz) {
    *siz = retsiz;
  }

  return (retstr);
}
