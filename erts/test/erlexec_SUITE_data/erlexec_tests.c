#if defined (__WIN32__)
int main() {return 0;}
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#define TRY(cmd) if ((cmd) < 0) bail_out(#cmd " failed")
static void bail_out(const char* msg)
{
  perror(msg);
  exit(-1);
}
static void alarm_handler(int signo)
{
  fprintf(stderr, __FILE__" self terminating after timeout\n");
  exit(1);
}
int main(int argc, char* argv[])
{
  pid_t child;
  int ret;
  char cmd;
  int child_exit;
  if (argc < 2) {
    fprintf(stderr, "Must specify command to run in background\n");
    exit(-1);
  }
  TRY(child=fork());
  if (child == 0) {
    pid_t gchild;
    TRY(setpgid(getpid(), getpid()));
    TRY(gchild=fork());
    if (gchild == 0) {
      TRY(execvp(argv[1],&argv[1]));
    }
    exit(0);
  }
  signal(SIGALRM, alarm_handler);
  alarm(10*60);
  TRY(wait(&child_exit));
  if (!WIFEXITED(child_exit) || WEXITSTATUS(child_exit)!=0) {
    fprintf(stderr, "child did not exit normally (status=%d)\n", child_exit);
    exit(-1);
  }
  for (;;)
    {
      TRY(ret=read(STDIN_FILENO, &cmd, 1));
      if (ret == 0) break;
      switch (cmd)
	{
	case 'K':
	  ret = kill(-child, SIGINT);
	  if (ret < 0 && errno != ESRCH) {
	    bail_out("kill failed");
	  }
 	  write(STDOUT_FILENO, &cmd, 1);
 	  break;
 	case '\n':
 	  break;
 	default:
 	  fprintf(stderr, "Unknown command '%c'\n", cmd);
	  exit(-1);
 	}
     }
  return 0;
}
#endif