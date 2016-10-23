#ifndef UTILS_H
#define UTILS_H 1

int
utGetDefaultGwInfo(char **szDevPtr, char **szIpPtr);

int
utGetIpFromNetDev(char *netdev, char **szIpPtr);

#ifdef HAVE_RESOLVE
int
utGetDomainInfo(char **szDefDomain, char **szDefServer);
#endif

#ifdef HAVE_UNAME
int
utGetUnameInfo(char **sysop, char **sysrelease, char **sysmachine);
#endif

#endif
