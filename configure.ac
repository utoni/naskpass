#                                               -*- Autoconf -*-
# configure script generation for ncblog
#

AC_PREREQ([2.67])
AC_INIT([naskpass], [0], [matzeton@googlemail.com])
AC_CONFIG_AUX_DIR([build])
AM_INIT_AUTOMAKE([1.11 subdir-objects foreign no-define -Wall -Werror])
AM_MAINTAINER_MODE([disable])
AM_WITH_DMALLOC
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_GNU_SOURCE
AC_CONFIG_SRCDIR([src/aconfig.h.in])
AC_CONFIG_HEADER([src/aconfig.h])
AC_USE_SYSTEM_EXTENSIONS

# Checks for programs.
AM_PROG_AR
AM_PROG_INSTALL_STRIP
AC_PROG_CC
AM_PROG_CC_C_O
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
AC_CHECK_HEADERS([glob.h stdio.h stdlib.h stdbool.h string.h unistd.h errno.h sys/stat.h sys/types.h sys/wait.h fcntl.h semaphore.h time.h mqueue.h syslog.h],[],[AC_MSG_ERROR([*** missing essential header files])])

have_resolv="false"
AC_CHECK_HEADERS([resolv.h],
                 [
                     AC_CHECK_LIB([resolv], [__res_init], [], [AC_MSG_ERROR([__res_init not found int resolv])])
                     AC_MSG_CHECKING([for working resolve])
                     AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <resolv.h>]],
                                                        [[res_init();]]
                                                        [[printf("%s\n", _res.defdname);]]
                                                        [[res_close();]]
                                                       )],
                                       [
                                            AC_MSG_RESULT([yes])
                                            have_resolv="true"
                                       ],
                                       [    AC_MSG_RESULT([no])])
                 ],[])
if test x"$have_resolv" = x"true" ; then
  AC_DEFINE_UNQUOTED([HAVE_RESOLVE], 1, [Define to 1 if you have libc resolv.])
  AM_CONDITIONAL([RESOLVE], true)
else
  AM_CONDITIONAL([RESOLVE], false)
  AC_MSG_WARN([*** DNS information disabled])
fi

have_uname="false"
AC_CHECK_HEADERS([sys/utsname.h],
                 [
                     AC_MSG_CHECKING([for working uname])
                     AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/utsname.h>]],
                                                        [[struct utsname nm;]]
                                                        [[uname(&nm);]]
                                                       )],
                                       [     
                                            AC_MSG_RESULT([yes])
                                            have_uname="true"
                                       ],
                                       [    AC_MSG_RESULT([no])])
                 ],[])
if test x"$have_uname" = x"true" ; then
  AC_DEFINE_UNQUOTED([HAVE_UNAME], 1, [Define to 1 if you have uname (sys/utsname.h) support.])
  AM_CONDITIONAL([UNAME], true)
else
  AM_CONDITIONAL([RESOLVE], false)
  AC_MSG_WARN([*** uname information disabled])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_COMPUTE_INT
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T
AC_CHECK_TYPE(size_t, unsigned int)
AC_CHECK_TYPE(ssize_t, unsigned int)
AC_CHECK_TYPE(time_t, unsigned int)
AC_CHECK_TYPE(pid_t, int)

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STRNLEN
AC_FUNC_STAT
AC_FUNC_MKTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([asprintf system printf fprintf mkfifo stat open close fork memmove memcpy memset strdup strndup strerror strstr strlen strnlen strtol openlog vsyslog closelog],,[AC_MSG_ERROR([*** Missing essential functions.])])

AC_MSG_CHECKING([for struct timespec])
AC_COMPILE_IFELSE( [AC_LANG_PROGRAM([[#include <time.h>]],
                                    [[struct timespec tm;]],
                                    [[clock_gettime(CLOCK_REALTIME, &tm);]],
                                    [[tm.tv_sec += 10;]])],
                   [
                      AC_MSG_RESULT([yes])
                      AC_DEFINE_UNQUOTED([HAVE_TIMESPEC], 1, [Define to 1 if you have struct timespec.])
                    ],
                    [
                      AC_MSG_RESULT([no])
                      AC_DEFINE_UNQUOTED([HAVE_TIMESPEC], 0, [Define to 1 if you have struct timespec.])
                    ]
                 )

AC_MSG_CHECKING([for struct stat])
AC_COMPILE_IFELSE( [AC_LANG_PROGRAM([[#include <sys/stat.h>]],
                                    [[struct stat st;]])],
                   [
                      AC_MSG_RESULT([yes])
                      AC_DEFINE_UNQUOTED([HAVE_STRUCTSTAT], 1, [Define to 1 if you have struct stat.])
                    ],
                    [
                      AC_MSG_RESULT([no])
                      AC_DEFINE_UNQUOTED([HAVE_STRUCTSTAT], 0, [Define to 1 if you have struct stat.])
                    ]
                 )

AC_MSG_CHECKING([for pthread/mutex/cond])
AC_COMPILE_IFELSE( [AC_LANG_PROGRAM([[#include <pthread.h>]],
                                    [[pthread_t thrd;]],
                                    [[pthread_cond_t cnd = PTHREAD_COND_INITIALIZER;]],
                                    [[pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;]])],
                   [
                      AC_MSG_RESULT([yes])
                      AC_DEFINE_UNQUOTED([HAVE_PTHREADS], 1, [Define to 1 if you have posix threads.])
                    ],
                    [
                      AC_MSG_RESULT([no])
                      AC_DEFINE_UNQUOTED([HAVE_PTHREADS], 0, [Define to 1 if you have posix threads.])
                    ]
                 )

AC_SEARCH_LIBS([initscr], [ncurses],,[AC_MSG_ERROR([*** Missing ncurses library. -lncurses])])
AC_SEARCH_LIBS([wattr_on], [ncurses],,[AC_MSG_ERROR([*** Missing ncurses library. -lncurses])])
AC_SEARCH_LIBS([wattr_off], [ncurses],,[AC_MSG_ERROR([*** Missing ncurses library. -lncurses])])
AC_SEARCH_LIBS([mvprintw], [ncurses],,[AC_MSG_ERROR([*** Missing ncurses library. -lncurses])])
AC_SEARCH_LIBS([mvwprintw], [ncurses],,[AC_MSG_ERROR([*** Missing ncurses library. -lncurses])])
AC_SEARCH_LIBS([wmove], [ncurses],,[AC_MSG_ERROR([*** Missing ncurses library. -lncurses])])
AC_SEARCH_LIBS([wgetch], [ncurses],,[AC_MSG_ERROR([*** Missing ncurses library. -lncurses])])
AC_SEARCH_LIBS([wtimeout], [ncurses],,[AC_MSG_ERROR([*** Missing ncurses library. -lncurses])])
AC_SEARCH_LIBS([wrefresh], [ncurses],,[AC_MSG_ERROR([*** Missing ncurses library. -lncurses])])
AC_SEARCH_LIBS([pthread_create], [pthread],,[AC_MSG_ERROR([*** Missing posix threads library. -lpthread])])
AC_SEARCH_LIBS([pthread_join], [pthread],,[AC_MSG_ERROR([*** Missing posix threads library. -lpthread])])
AC_SEARCH_LIBS([pthread_mutex_lock], [pthread],,[AC_MSG_ERROR([*** Missing posix threads library. -lpthread])])
AC_SEARCH_LIBS([pthread_mutex_unlock], [pthread],,[AC_MSG_ERROR([*** Missing posix threads library. -lpthread])])
AC_SEARCH_LIBS([pthread_cond_timedwait], [pthread],,[AC_MSG_ERROR([*** Missing posix threads library. -lpthread])])
AC_SEARCH_LIBS([pthread_cond_signal], [pthread],,[AC_MSG_ERROR([*** Missing posix threads library. -lpthread])])
AC_SEARCH_LIBS([sem_open], [pthread],,[AC_MSG_ERROR([*** Missing posix semaphores. -lpthread])])
AC_SEARCH_LIBS([sem_close], [pthread],,[AC_MSG_ERROR([*** Missing posix semaphores. -lpthread])])
AC_SEARCH_LIBS([sem_unlink], [pthread],,[AC_MSG_ERROR([*** Missing posix semaphores. -lpthread])])
AC_SEARCH_LIBS([sem_post], [pthread],,[AC_MSG_ERROR([*** Missing posix semaphores. -lpthread])])
AC_SEARCH_LIBS([sem_wait], [pthread],,[AC_MSG_ERROR([*** Missing posix semaphores. -lpthread])])
AC_SEARCH_LIBS([sem_trywait], [pthread],,[AC_MSG_ERROR([*** Missing posix semaphores. -lpthread])])
AC_SEARCH_LIBS([sem_getvalue], [pthread],,[AC_MSG_ERROR([*** Missing posix semaphores. -lpthread])])
AC_SEARCH_LIBS([mq_open], [rt],,[AC_MSG_ERROR([*** Missing posix ipc library. -lrt])])
AC_SEARCH_LIBS([mq_close], [rt],,[AC_MSG_ERROR([*** Missing posix ipc library. -lrt])])
AC_SEARCH_LIBS([mq_unlink], [rt],,[AC_MSG_ERROR([*** Missing posix ipc library. -lrt])])
AC_SEARCH_LIBS([clock_gettime], [rt],,[AC_MSG_ERROR([*** Missing posix ipc library. -lrt])])

AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])
AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

AC_DEFINE([HAVE_CONFIG], [1], [Do NOT change THIS!])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_LDFLAGS])
AC_SUBST([AM_CPPFLAGS])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

echo "Run 'make' to finish the process."
test -d .git && VERSION="git-$(git rev-parse --short HEAD)"
echo "#define VERSION \"${VERSION}\"" >src/version.h

