#                                               -*- Autoconf -*-
# configure script generation for ncblog
#

AC_PREREQ([2.67])
AC_INIT([naskpass], [0], [matzeton@googlemail.com])
AC_CONFIG_AUX_DIR([build])
AM_INIT_AUTOMAKE([1.11 foreign no-define -Wall -Werror])
AM_WITH_DMALLOC
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CONFIG_SRCDIR([src/aconfig.h.in])
AC_CONFIG_HEADER([src/aconfig.h])
AC_CONFIG_MACRO_DIR([m4])
AC_USE_SYSTEM_EXTENSIONS
CFLAGS="-Os"
LDFLAGS=""

# Checks for programs.
AM_PROG_AR
AM_PROG_INSTALL_STRIP
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_C_INLINE
AC_PREFIX_DEFAULT([/usr])
AC_CHECK_TOOL([STRIP],[strip])

# Checks for header files.
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_HEADER_STDBOOL
AC_HEADER_STDC
AC_HEADER_STAT
AC_HEADER_DIRENT
AC_HEADER_ASSERT
AC_CHECK_HEADERS([stdio.h stdlib.h stdbool.h string.h unistd.h errno.h sys/stat.h sys/types.h sys/wait.h fcntl.h semaphore.h time.h mqueue.h alloca.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_COMPUTE_INT
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T
AX_CHECK_TYPEDEF([size_t], [stdio.h],,[AC_MSG_ERROR([*** Missing size_t typedef in stdio.h])])
AX_CHECK_TYPEDEF([ssize_t], [stdio.h],,[AC_MSG_ERROR([*** Missing size_t typedef in stdio.h])])

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STRNLEN
AC_FUNC_STAT
AC_FUNC_MKTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([clock_gettime asprintf system printf fprintf mkfifo stat open close fork memmove memcpy memset strdup strndup strerror strstr strlen strnlen strtol],,[AC_MSG_ERROR([*** Missing essential functions.])])

AC_DEFINE([HAVE_CONFIG], [1], [Do NOT change THIS!])
LDFLAGS="${LDFLAGS} -pthread -lrt -lncurses"
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_LDFLAGS])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

echo "Run 'make' to finish the process."
test -d .git && VERSION="git-$(git rev-parse --short HEAD)"
echo "#define VERSION \"${VERSION}\"" >src/version.h

