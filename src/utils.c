#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "config.h"

#ifdef HAVE_RESOLVE
#include <resolv.h>
#endif
#ifdef HAVE_UNAME
#include <sys/utsname.h>
#endif


int
utGetDefaultGwInfo(char **szDevPtr, char **szIpPtr)
{
  FILE *f;
  char line[100];
  char *p, *c, *g;
  char *saveptr;
  int ret = 1;

  f = fopen("/proc/net/route" , "r");
  while (fgets(line , 100 , f)) {
    p = strtok_r(line , " \t", &saveptr);
    c = strtok_r(NULL , " \t", &saveptr);
    g = strtok_r(NULL , " \t", &saveptr);

    if (p && c) {
      if (strcmp(c , "00000000") == 0) {
        *szDevPtr = calloc(IFNAMSIZ, sizeof(char));
        memcpy(*szDevPtr, p, IFNAMSIZ-1);
        *szIpPtr = NULL;
        if (g) {
          char *pEnd;
          int ng = strtol(g,&pEnd,16);
          struct in_addr addr;
          addr.s_addr = ng;
          *szIpPtr = calloc(IFNAMSIZ, sizeof(char));
          memcpy(*szIpPtr, inet_ntoa(addr), IFNAMSIZ-1);
          ret = 0;
        }
        break;
      }
    }
  }
  fclose(f);
  return ret;
}

int
utGetIpFromNetDev(char *netdev, char **szIpPtr)
{
  int fd;
  struct ifreq ifr;
  int ret = 1;

  memset(&ifr, '\0', sizeof(struct ifreq));
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, netdev, IFNAMSIZ-1);
  if (ioctl(fd, SIOCGIFADDR, &ifr) != -1) {
    *szIpPtr = calloc(IFNAMSIZ, sizeof(char));
    memcpy(*szIpPtr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), IFNAMSIZ-1);
    ret = 0;
  }
  if (fd >= 0)
    close(fd);
  return ret;
}

#ifdef HAVE_RESOLVE
int
utGetDomainInfo(char **szDefDomain, char **szDefServer)
{
  if (res_init() != 0)
    return 1;
  *szDefDomain = calloc(257, sizeof(char));
  memcpy(*szDefDomain, _res.defdname, 256);
  if (strnlen(*szDefDomain, 256) == 0)
    *szDefDomain[0] = '-';
  *szDefServer = calloc(IFNAMSIZ, sizeof(char));
  memcpy(*szDefServer, inet_ntoa(_res.nsaddr_list[0].sin_addr), IFNAMSIZ-1);
  res_close();
  return 0;
}
#endif

#ifdef HAVE_UNAME
int
utGetUnameInfo(char **sysop, char **sysrelease, char **sysmachine) {
  struct utsname un;

  memset(&un, '\0', sizeof(struct utsname));
  if (uname(&un) != 0)
    return 1;

  *sysop = calloc(_UTSNAME_SYSNAME_LENGTH, sizeof(char));
  memcpy(*sysop, un.sysname, _UTSNAME_SYSNAME_LENGTH);
  *sysrelease = calloc(_UTSNAME_RELEASE_LENGTH, sizeof(char));
  memcpy(*sysrelease, un.release, _UTSNAME_RELEASE_LENGTH);
  *sysmachine = calloc(_UTSNAME_MACHINE_LENGTH, sizeof(char));
  memcpy(*sysmachine, un.machine, _UTSNAME_MACHINE_LENGTH);
  return 0;
}
#endif
