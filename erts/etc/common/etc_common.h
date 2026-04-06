#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#if !defined(__WIN32__)
#  include <dirent.h>
#  include <limits.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <unistd.h>
#else
#  include <windows.h>
#  include <io.h>
#  include <winbase.h>
#  include <process.h>
#  include <direct.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifndef MAXPATHLEN
#   ifdef PATH_MAX
#       define MAXPATHLEN PATH_MAX
#   else
#       define MAXPATHLEN 2048
#   endif
#endif
#include "erl_printf.h"
#ifdef __WIN32__
#define HAVE_STRERROR 1
#define snprintf _snprintf
#endif
#ifdef __IOS__
#ifdef system
#undef system
#endif
#define system(X) 0
#endif
#ifdef DEBUG
#  define ASSERT(Cnd) ((void)((Cnd) ? 1 : abort()))
#else
#  define ASSERT(Cnd)
#endif