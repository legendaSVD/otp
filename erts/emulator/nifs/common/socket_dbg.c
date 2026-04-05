#ifdef HAVE_CONFIG_H
#    include "config.h"
#endif
#ifdef ESOCK_ENABLE
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <erl_nif.h>
#include "socket_util.h"
#include "socket_dbg.h"
#define TSELF()            enif_thread_self()
#define TNAME(__T__)       enif_thread_name( __T__ )
#define TSNAME()           TNAME(TSELF())
FILE* esock_dbgout = NULL;
#define PATH_LEN (MAX_PATH - 14)
extern
BOOLEAN_T esock_dbg_init(char* filename)
{
    size_t n;
    FILE *fp;
    const char mode[] = "w+";
    if (filename == NULL) {
        esock_dbgout = stdout;
        return TRUE;
    }
    if ((n = strlen(filename)) == 0) {
        esock_dbgout = stdout;
        return TRUE;
    }
    fp = NULL;
#if defined(__WIN32__)
    {
        fp = fopen(filename, mode);
    }
#else
    if (n >= 6) {
        size_t k;
        for (k = n - 6;  k < n;  k++)
            if (filename[k] != '?') break;
        if (k == n) {
            int fd;
            for (k = n - 6;  k < n;  k++)
                filename[k] = 'X';
            if ((fd = mkstemp(filename)) >= 0)
                fp = fdopen(fd, mode);
        } else {
            fp = fopen(filename, mode);
        }
    } else {
        fp = fopen(filename, mode);
    }
#endif
    if (fp != NULL) {
        esock_dbgout = fp;
        return TRUE;
    }
    esock_dbgout = stdout;
    return FALSE;
}
extern
void esock_dbg_printf( const char* prefix, const char* format, ... )
{
  va_list         args;
  char            f[512];
  char            stamp[64];
  int             res;
  if (esock_timestamp_str(stamp, sizeof(stamp))) {
      res = enif_snprintf(f, sizeof(f), "%s [%s] [%s] %s",
                          prefix, stamp, TSNAME(), format);
  } else {
      res = enif_snprintf(f, sizeof(f), "%s [%s] %s",
                          prefix, TSNAME(), format);
  }
  if (res < sizeof(f)) {
      va_start (args, format);
      enif_vfprintf(esock_dbgout, f, args);
      va_end (args);
      fflush(esock_dbgout);
  }
}
#endif