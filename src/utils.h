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

#endif
