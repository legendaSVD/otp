#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifndef __WIN32__
#  include <unistd.h>
#  include <sys/time.h>
#else
#  include "windows.h"
#  include "winbase.h"
#endif
#include <sys/types.h>
int
main(int argc, char *argv[])
{
    long int ms;
    char *endp;
    if (argc != 2) {
        fprintf(stderr, "Invalid argument count: %d\n", argc);
        exit(1);
    }
    errno = 0;
    ms = strtol(argv[1], &endp, 10);
    if (errno || argv[1] == endp || *endp != '\0' || ms < 0) {
        if (errno == 0)
            errno = EINVAL;
        perror("Invalid timeout value");
        exit(1);
    }
#ifdef __WIN32__
    Sleep(ms);
#else
    {
        struct timeval t;
        t.tv_sec = ms/1000;
        t.tv_usec = (ms % 1000) * 1000;
        select(0, NULL, NULL, NULL, &t);
    }
#endif
  return 0;
}