#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#ifdef HAVE_WORKING_POSIX_OPENPT
#  ifndef _XOPEN_SOURCE
#    if !(defined(__APPLE__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__) || defined(__sun))
#      define _XOPEN_SOURCE 600
#    endif
#  endif
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <termios.h>
#include <time.h>
#ifdef HAVE_SYSLOG_H
#  include <syslog.h>
#endif
#ifdef HAVE_PTY_H
#  include <pty.h>
#endif
#ifdef HAVE_UTMP_H
#  include <utmp.h>
#endif
#ifdef HAVE_LIBUTIL_H
#  include <libutil.h>
#endif
#ifdef HAVE_UTIL_H
#  include <util.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#  include <sys/ioctl.h>
#endif
#if defined(__sun) && defined(__SVR4)
#  include <stropts.h>
#endif
#ifdef __clang_analyzer__
#  undef FD_ZERO
#  define FD_ZERO(FD_SET_PTR) memset(FD_SET_PTR, 0, sizeof(fd_set))
#endif
#include "run_erl.h"
#include "safe_string.h"
#ifdef O_NONBLOCK
#  define DONT_BLOCK_PLEASE O_NONBLOCK
#else
#  define DONT_BLOCK_PLEASE O_NDELAY
#  ifndef EAGAIN
#    define EAGAIN -3898734
#  endif
#endif
#define noDEBUG
#define DEFAULT_LOG_GENERATIONS 5
#define LOG_MAX_GENERATIONS     1000
#define LOG_MIN_GENERATIONS     2
#define DEFAULT_LOG_MAXSIZE     100000
#define LOG_MIN_MAXSIZE         1000
#define LOG_STUBNAME            "erlang.log."
#define LOG_PERM                0664
#define DEFAULT_LOG_ACTIVITY_MINUTES    5
#define DEFAULT_LOG_ALIVE_MINUTES       15
#define DEFAULT_LOG_ALIVE_FORMAT        "%a %b %e %T %Z %Y"
#define ALIVE_BUFFSIZ                   256
#define PERM            0600
#define STATUSFILENAME  "/run_erl.log"
#define PIPE_STUBNAME   "erlang.pipe"
#define PIPE_STUBLEN    strlen(PIPE_STUBNAME)
#ifndef FILENAME_MAX
#define FILENAME_MAX 250
#endif
#ifndef O_SYNC
#define O_SYNC 0
#define USE_FSYNC 1
#endif
#define MAX(x,y)  ((x) > (y) ? (x) : (y))
#define FILENAME_BUFSIZ FILENAME_MAX
static void usage(char *);
static int create_fifo(char *name, int perm);
static int open_pty_master(char **name, int *sfd);
static int open_pty_slave(char *name);
static void pass_on(pid_t);
static void exec_shell(char **);
static void status(const char *format,...);
static void error_logf(int priority, int line, const char *format,...);
static void catch_sigchild(int);
static int next_log(int log_num);
static int prev_log(int log_num);
static int find_next_log_num(void);
static int open_log(int log_num, int flags);
static void write_to_log(int* lfd, int* log_num, char* buf, int len);
static void daemon_init(void);
static char *simple_basename(char *path);
static void init_outbuf(void);
static int outbuf_size(void);
static void clear_outbuf(void);
static char* outbuf_first(void);
static void outbuf_delete(int bytes);
static void outbuf_append(const char* bytes, int n);
static int write_all(int fd, const char* buf, int len);
static int extract_ctrl_seq(char* buf, int len);
static void set_window_size(unsigned col, unsigned row);
static ssize_t sf_write(int fd, const void *buffer, size_t len);
static ssize_t sf_read(int fd, void *buffer, size_t len);
static int sf_open(const char *path, int flags, mode_t mode);
static int sf_close(int fd);
#ifdef DEBUG
static void show_terminal_settings(struct termios *t);
#endif
static char fifo1[FILENAME_BUFSIZ], fifo2[FILENAME_BUFSIZ];
static char statusfile[FILENAME_BUFSIZ];
static char log_dir[FILENAME_BUFSIZ];
static char pipename[FILENAME_BUFSIZ];
static FILE *stdstatus = NULL;
static int log_generations = DEFAULT_LOG_GENERATIONS;
static int log_maxsize     = DEFAULT_LOG_MAXSIZE;
static int log_alive_minutes = DEFAULT_LOG_ALIVE_MINUTES;
static int log_activity_minutes = DEFAULT_LOG_ACTIVITY_MINUTES;
static int log_alive_in_gmt = 0;
static char log_alive_format[ALIVE_BUFFSIZ+1];
static int run_daemon = 0;
static char *program_name;
static int mfd;
static unsigned protocol_ver = RUN_ERL_LO_VER;
static char* outbuf_base;
static int outbuf_total;
static char* outbuf_out;
static char* outbuf_in;
#if defined(NO_SYSCONF) || !defined(_SC_OPEN_MAX)
#    if defined(OPEN_MAX)
#        define HIGHEST_FILENO() OPEN_MAX
#    else
#        define HIGHEST_FILENO() 64
#    endif
#else
#    define HIGHEST_FILENO() sysconf(_SC_OPEN_MAX)
#endif
#ifndef HAVE_SYSLOG_H
#    define OPEN_SYSLOG() ((void) 0)
#    define LOG_ERR NULL
#else
#    define OPEN_SYSLOG() openlog(simple_basename(program_name),   \
                                  LOG_PID|LOG_CONS|LOG_NOWAIT,LOG_USER)
#endif
#define ERROR0(Prio,Format) error_logf(Prio,__LINE__,Format"\n")
#define ERROR1(Prio,Format,A1) error_logf(Prio,__LINE__,Format"\n",A1)
#define ERROR2(Prio,Format,A1,A2) error_logf(Prio,__LINE__,Format"\n",A1,A2)
#ifdef HAVE_STRERROR
#    define ADD_ERRNO(Format) "errno=%d '%s'\n"Format"\n",errno,strerror(errno)
#else
#    define ADD_ERRNO(Format) "errno=%d\n"Format"\n",errno
#endif
#define ERRNO_ERR0(Prio,Format) error_logf(Prio,__LINE__,ADD_ERRNO(Format))
#define ERRNO_ERR1(Prio,Format,A1) error_logf(Prio,__LINE__,ADD_ERRNO(Format),A1)
int main(int argc, char **argv)
{
  int childpid;
  int sfd = -1;
  int fd;
  char *p, *ptyslave=NULL;
  int i = 1;
  int off_argv;
  int calculated_pipename = 0;
  int highest_pipe_num = 0;
  int sleepy_child = 0;
  program_name = argv[0];
  if(argc<4) {
    usage(argv[0]);
    exit(1);
  }
  init_outbuf();
  if (!strcmp(argv[1],"-sleepy-child")) {
      sleepy_child = 1;
      ++i;
  }
  if (!strcmp(argv[1],"-daemon")) {
      daemon_init();
      ++i;
  }
  off_argv = i;
  strn_cpy(pipename, sizeof(pipename), argv[i++]);
  strn_cpy(log_dir, sizeof(log_dir), argv[i]);
  strn_cpy(statusfile, sizeof(statusfile), log_dir);
  strn_cat(statusfile, sizeof(statusfile), STATUSFILENAME);
#ifdef DEBUG
  status("%s: pid is : %d\n", argv[0], getpid());
#endif
  if ((p = getenv("RUN_ERL_LOG_ALIVE_MINUTES"))) {
      log_alive_minutes = atoi(p);
      if (!log_alive_minutes) {
	  ERROR1(LOG_ERR,"Minimum value for RUN_ERL_LOG_ALIVE_MINUTES is 1 "
		 "(current value is %s)",p);
      }
      log_activity_minutes = log_alive_minutes / 3;
      if (!log_activity_minutes) {
	  ++log_activity_minutes;
      }
  }
  if ((p = getenv("RUN_ERL_LOG_ACTIVITY_MINUTES"))) {
     log_activity_minutes = atoi(p);
      if (!log_activity_minutes) {
	  ERROR1(LOG_ERR,"Minimum value for RUN_ERL_LOG_ACTIVITY_MINUTES is 1 "
		 "(current value is %s)",p);
      }
  }
  if ((p = getenv("RUN_ERL_LOG_ALIVE_FORMAT"))) {
      if (strlen(p) > ALIVE_BUFFSIZ) {
	  ERROR1(LOG_ERR, "RUN_ERL_LOG_ALIVE_FORMAT can contain a maximum of "
		 "%d characters", ALIVE_BUFFSIZ);
      }
      strn_cpy(log_alive_format, sizeof(log_alive_format), p);
  } else {
      strn_cpy(log_alive_format, sizeof(log_alive_format), DEFAULT_LOG_ALIVE_FORMAT);
  }
  if ((p = getenv("RUN_ERL_LOG_ALIVE_IN_UTC")) && strcmp(p,"0")) {
      ++log_alive_in_gmt;
  }
  if ((p = getenv("RUN_ERL_LOG_GENERATIONS"))) {
    log_generations = atoi(p);
    if (log_generations < LOG_MIN_GENERATIONS)
      ERROR1(LOG_ERR,"Minimum RUN_ERL_LOG_GENERATIONS is %d", LOG_MIN_GENERATIONS);
    if (log_generations > LOG_MAX_GENERATIONS)
      ERROR1(LOG_ERR,"Maximum RUN_ERL_LOG_GENERATIONS is %d", LOG_MAX_GENERATIONS);
  }
  if ((p = getenv("RUN_ERL_LOG_MAXSIZE"))) {
    log_maxsize = atoi(p);
    if (log_maxsize < LOG_MIN_MAXSIZE)
      ERROR1(LOG_ERR,"Minimum RUN_ERL_LOG_MAXSIZE is %d", LOG_MIN_MAXSIZE);
  }
  if(*pipename && pipename[strlen(pipename)-1] == '/') {
    DIR *dirp;
    struct dirent *direntp;
    calculated_pipename = 1;
    dirp = opendir(pipename);
    if(!dirp) {
      ERRNO_ERR1(LOG_ERR,"Can't access pipe directory '%s'.", pipename);
      exit(1);
    }
    while((direntp=readdir(dirp)) != NULL) {
      if(strncmp(direntp->d_name,PIPE_STUBNAME,PIPE_STUBLEN)==0) {
	int num = atoi(direntp->d_name+PIPE_STUBLEN+1);
	if(num > highest_pipe_num)
	  highest_pipe_num = num;
      }
    }
    closedir(dirp);
    strn_catf(pipename, sizeof(pipename), "%s.%d",
	      PIPE_STUBNAME, highest_pipe_num+1);
  }
  for(;;) {
      strn_cpy(fifo1, sizeof(fifo1), pipename);
      strn_cat(fifo1, sizeof(fifo1), ".r");
      if (create_fifo(fifo1, PERM) < 0) {
	  ERRNO_ERR1(LOG_ERR,"Cannot create FIFO %s for writing.", fifo1);
	  exit(1);
      }
      strn_cpy(fifo2, sizeof(fifo2), pipename);
      strn_cat(fifo2, sizeof(fifo2), ".w");
      if ((fd = sf_open(fifo2, O_WRONLY|DONT_BLOCK_PLEASE, 0)) >= 0) {
	  sf_close(fd);
	  if (calculated_pipename) {
	      ++highest_pipe_num;
	      strn_catf(pipename, sizeof(pipename), "%s.%d",
			PIPE_STUBNAME, highest_pipe_num+1);
	      continue;
	  }
	  fprintf(stderr, "Erlang already running on pipe %s.\n", pipename);
	  exit(1);
      }
      if (create_fifo(fifo2, PERM) < 0) {
	  ERRNO_ERR1(LOG_ERR,"Cannot create FIFO %s for reading.", fifo2);
	  exit(1);
      }
      break;
  }
  if ((mfd = open_pty_master(&ptyslave, &sfd)) < 0) {
    ERRNO_ERR0(LOG_ERR,"Could not open pty master");
    exit(1);
  }
  if ((childpid = fork()) < 0) {
    ERRNO_ERR0(LOG_ERR,"Cannot fork");
    exit(1);
  }
  if (childpid == 0) {
      if (sleepy_child)
	  sleep(1);
    sf_close(mfd);
#ifdef USE_SETPGRP_NOARGS
    setpgrp();
#elif defined(USE_SETPGRP)
    setpgrp(0,getpid());
#else
    setsid();
#endif
    if (sfd < 0) {
	if ((sfd = open_pty_slave(ptyslave)) < 0) {
	    ERRNO_ERR1(LOG_ERR,"Could not open pty slave '%s'", ptyslave);
	    exit(1);
	}
	while (sfd < 3) {
	    sfd = dup(sfd);
	}
    }
#if defined(HAVE_OPENPTY) && defined(TIOCSCTTY)
    else {
	ioctl(sfd, TIOCSCTTY, (char *)NULL);
    }
#endif
#ifdef HAVE_SYSLOG_H
    if (run_daemon) {
	closelog();
    }
#endif
    sf_close(0);
    sf_close(1);
    sf_close(2);
    if (dup(sfd) != 0 || dup(sfd) != 1 || dup(sfd) != 2) {
      status("Cannot dup\n");
    }
    sf_close(sfd);
    exec_shell(argv+off_argv);
  } else {
    struct sigaction sig_act;
    sigemptyset(&sig_act.sa_mask);
    sig_act.sa_flags = 0;
    sig_act.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sig_act, (struct sigaction *)NULL);
    sigemptyset(&sig_act.sa_mask);
    sig_act.sa_flags = SA_NOCLDSTOP;
    sig_act.sa_handler = catch_sigchild;
    sigaction(SIGCHLD, &sig_act, (struct sigaction *)NULL);
    pass_on(childpid);
  }
  return 0;
}
#ifdef HAVE_GCC_DIAG_IGNORE_WFORMAT_NONLITERAL
_Pragma("GCC diagnostic push");
_Pragma("GCC diagnostic ignored \"-Wformat-nonliteral\"");
#endif
static int dynamic_strftime(
    char *__restrict__ log_alive_buffer,
    const char *__restrict__ log_alive_format,
    const struct tm *__restrict__ tmptr) {
    return strftime(log_alive_buffer, ALIVE_BUFFSIZ, log_alive_format, tmptr);
}
#ifdef HAVE_GCC_DIAG_IGNORE_WFORMAT_NONLITERAL
_Pragma("GCC diagnostic pop");
#endif
static void pass_on(pid_t childpid)
{
    int len;
    fd_set readfds;
    fd_set writefds;
    fd_set* writefds_ptr;
    struct timeval timeout;
    time_t last_activity;
    char buf[BUFSIZ];
    char log_alive_buffer[ALIVE_BUFFSIZ+1];
    int lognum;
    int rfd, wfd=0, lfd=0;
    int maxfd;
    int ready;
    int got_some = 0;
    if ((rfd = sf_open(fifo2, O_RDONLY|DONT_BLOCK_PLEASE, 0)) < 0) {
	ERRNO_ERR1(LOG_ERR,"Could not open FIFO '%s' for reading.", fifo2);
	exit(1);
    }
#ifdef DEBUG
    status("run_erl: %s opened for reading\n", fifo2);
#endif
    lognum = find_next_log_num();
    lfd = open_log(lognum, O_RDWR|O_APPEND|O_CREAT|O_SYNC);
    while (1) {
	int exit_status;
	maxfd = MAX(rfd, mfd);
	maxfd = MAX(wfd, maxfd);
	FD_ZERO(&readfds);
	FD_SET(rfd, &readfds);
	FD_SET(mfd, &readfds);
	FD_ZERO(&writefds);
	if (outbuf_size() == 0) {
	    writefds_ptr = NULL;
	} else {
	    FD_SET(wfd, &writefds);
	    writefds_ptr = &writefds;
	}
	time(&last_activity);
	timeout.tv_sec  = log_alive_minutes*60;
	timeout.tv_usec = 0;
	ready = select(maxfd + 1, &readfds, writefds_ptr, NULL, &timeout);
	if (ready < 0) {
	    if (errno == EINTR) {
		if (waitpid(childpid, &exit_status, WNOHANG) == childpid) {
		    alarm(5);
		}
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
	    } else {
		ERRNO_ERR0(LOG_ERR,"Error in select.");
		exit(1);
	    }
	} else {
	    time_t now;
	    if (waitpid(childpid, &exit_status, WNOHANG) == childpid) {
		alarm(5);
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
	    }
	    time(&now);
	    if(!ready || now - last_activity > log_activity_minutes*60) {
		struct tm *tmptr;
		if (log_alive_in_gmt) {
		    tmptr = gmtime(&now);
		} else {
		    tmptr = localtime(&now);
		}
		if (!dynamic_strftime(log_alive_buffer, log_alive_format, tmptr)) {
		    strn_cpy(log_alive_buffer, sizeof(log_alive_buffer),
			     "(could not format time in 256 positions "
			     "with current format string.)");
		}
		log_alive_buffer[ALIVE_BUFFSIZ] = '\0';
		sn_printf(buf, sizeof(buf), "\n===== %s%s\n",
			  ready?"":"ALIVE ", log_alive_buffer);
		write_to_log(&lfd, &lognum, buf, strlen(buf));
	    }
	}
	if (FD_ISSET(wfd, &writefds)) {
	    int written;
	    char* buf = outbuf_first();
	    len = outbuf_size();
	    written = sf_write(wfd, buf, len);
	    if (written < 0 && errno == EAGAIN) {
	    } else if (written < 0) {
		clear_outbuf();
		sf_close(wfd);
		wfd = 0;
	    } else {
		outbuf_delete(written);
	    }
	}
	if (FD_ISSET(mfd, &readfds)) {
#ifdef DEBUG
	    status("Pty master read; ");
#endif
	    if ((len = sf_read(mfd, buf, BUFSIZ)) <= 0) {
		int saved_errno = errno;
		sf_close(rfd);
		if(wfd) sf_close(wfd);
		sf_close(mfd);
		unlink(fifo1);
		unlink(fifo2);
		if (len < 0) {
		    errno = saved_errno;
		    if(errno == EIO)
			ERROR0(LOG_ERR,"Erlang closed the connection.");
		    else
			ERRNO_ERR0(LOG_ERR,"Error in reading from terminal");
		    exit(1);
		}
		exit(0);
	    }
	    write_to_log(&lfd, &lognum, buf, len);
	    if (wfd) {
		outbuf_append(buf, len);
	    }
	}
	if (FD_ISSET(rfd, &readfds)) {
#ifdef DEBUG
	    status("FIFO read; ");
#endif
	    if ((len = sf_read(rfd, buf, BUFSIZ)) < 0) {
		sf_close(rfd);
		if(wfd) sf_close(wfd);
		sf_close(mfd);
		unlink(fifo1);
		unlink(fifo2);
		ERRNO_ERR0(LOG_ERR,"Error in reading from FIFO.");
		exit(1);
	    }
	    if(!len) {
		sf_close(rfd);
		rfd = sf_open(fifo2, O_RDONLY|DONT_BLOCK_PLEASE, 0);
		if (rfd < 0) {
		    ERRNO_ERR1(LOG_ERR,"Could not open FIFO '%s' for reading.", fifo2);
		    exit(1);
		}
		got_some = 0;
	    }
	    else {
		if(!wfd) {
		    if ((wfd = sf_open(fifo1, O_WRONLY|DONT_BLOCK_PLEASE, 0)) < 0) {
			status("Client expected on FIFO %s, but can't open (len=%d)\n",
			       fifo1, len);
			sf_close(rfd);
			rfd = sf_open(fifo2, O_RDONLY|DONT_BLOCK_PLEASE, 0);
			if (rfd < 0) {
			    ERRNO_ERR1(LOG_ERR,"Could not open FIFO '%s' for reading.", fifo2);
			    exit(1);
			}
			wfd = 0;
		    }
		    else {
#ifdef DEBUG
			status("run_erl: %s opened for writing\n", fifo1);
#endif
		    }
		}
    if (!got_some && wfd && buf[0] == '\033' &&
         buf[1] == 'l') {
		    char wbuf[30];
		    int wlen = sn_printf(wbuf,sizeof(wbuf),"[run_erl v%u-%u]\n",
					 RUN_ERL_HI_VER, RUN_ERL_LO_VER);
		    outbuf_append(wbuf,wlen);
		}
		got_some = 1;
#ifdef DEBUG
		status("Pty master write; ");
#endif
		len = extract_ctrl_seq(buf, len);
		if(len==1 && buf[0] == '\003') {
		    kill(childpid,SIGINT);
		}
		else if (len>0 && write_all(mfd, buf, len) != len) {
		    ERRNO_ERR0(LOG_ERR,"Error in writing to terminal.");
		    sf_close(rfd);
		    if(wfd) sf_close(wfd);
		    sf_close(mfd);
		    exit(1);
		}
	    }
#ifdef DEBUG
	    status("OK\n");
#endif
	}
    }
}
static void catch_sigchild(int sig)
{
}
static int next_log(int log_num) {
  return log_num>=log_generations?1:log_num+1;
}
static int prev_log(int log_num) {
  return log_num<=1?log_generations:log_num-1;
}
static int find_next_log_num(void) {
  int i, next_gen, log_gen;
  DIR *dirp;
  struct dirent *direntp;
  int log_exists[LOG_MAX_GENERATIONS+1];
  int stub_len = strlen(LOG_STUBNAME);
  for(i=log_generations; i>=0; i--)
    log_exists[i] = 0;
  dirp = opendir(log_dir);
  if(!dirp) {
    ERRNO_ERR1(LOG_ERR,"Can't access log directory '%s'", log_dir);
    exit(1);
  }
  while((direntp=readdir(dirp)) != NULL) {
    if(strncmp(direntp->d_name,LOG_STUBNAME,stub_len)==0) {
      int num = atoi(direntp->d_name+stub_len);
      if(num < 1 || num > log_generations)
	continue;
      log_exists[num] = 1;
    }
  }
  closedir(dirp);
  next_gen = 0;
  for(i=log_generations; i>=0; i--) {
    if(log_exists[i])
      if(next_gen)
	break;
      else
	;
    else
      next_gen = i;
  }
  if(next_gen)
    log_gen = prev_log(next_gen);
  else
    log_gen = 1;
  return log_gen;
}
static int open_log(int log_num, int flags)
{
  char buf[FILENAME_MAX];
  time_t now;
  struct tm *tmptr;
  char log_buffer[ALIVE_BUFFSIZ+1];
  int lfd;
  sn_printf(buf, sizeof(buf), "%s/%s%d",
	    log_dir, LOG_STUBNAME, next_log(log_num));
  unlink(buf);
  sn_printf(buf, sizeof(buf), "%s/%s%d", log_dir, LOG_STUBNAME, log_num);
  if((lfd = sf_open(buf, flags, LOG_PERM))<0){
      ERRNO_ERR1(LOG_ERR,"Can't open log file '%s'.", buf);
    exit(1);
  }
  time(&now);
  if (log_alive_in_gmt) {
      tmptr = gmtime(&now);
  } else {
      tmptr = localtime(&now);
  }
  if (!dynamic_strftime(log_buffer, log_alive_format, tmptr)) {
      strn_cpy(log_buffer, sizeof(log_buffer),
	      "(could not format time in 256 positions "
	      "with current format string.)");
  }
  log_buffer[ALIVE_BUFFSIZ] = '\0';
  sn_printf(buf, sizeof(buf), "\n=====\n===== LOGGING STARTED %s\n=====\n",
	    log_buffer);
  if (write_all(lfd, buf, strlen(buf)) < 0)
      status("Error in writing to log.\n");
#ifdef USE_FSYNC
  fsync(lfd);
#endif
  return lfd;
}
static void write_to_log(int* lfd, int* log_num, char* buf, int len)
{
  int size;
  size = lseek(*lfd,0,SEEK_END);
  if(size+len > log_maxsize) {
    sf_close(*lfd);
    *log_num = next_log(*log_num);
    *lfd = open_log(*log_num, O_RDWR|O_CREAT|O_TRUNC|O_SYNC);
  }
  if (write_all(*lfd, buf, len) < 0) {
    status("Error in writing to log.\n");
  }
#ifdef USE_FSYNC
  fsync(*lfd);
#endif
}
static int create_fifo(char *name, int perm)
{
  if ((mkfifo(name, perm) < 0) && (errno != EEXIST))
    return -1;
  return 0;
}
#ifdef HAVE_WORKING_POSIX_OPENPT
#  if !(defined(__OpenBSD__) && defined(HAVE_OPENPTY))
#    define TRY_POSIX_OPENPT
#  endif
#endif
static int open_pty_master(char **ptyslave, int *sfdp)
{
  int mfd;
#if defined(TRY_POSIX_OPENPT) || (defined(__sun) && defined(__SVR4))
#  ifdef TRY_POSIX_OPENPT
  mfd = posix_openpt(O_RDWR);
#  elif defined(__sun) && defined(__SVR4)
  mfd = sf_open("/dev/ptmx", O_RDWR, 0);
#  endif
  if (mfd >= 0) {
      if ((*ptyslave = ptsname(mfd)) != NULL &&
	  grantpt(mfd) == 0 &&
	  unlockpt(mfd) == 0) {
	  return mfd;
      }
      sf_close(mfd);
  }
#endif
#if defined(HAVE_OPENPTY)
#  ifdef PATH_MAX
#    define SLAVE_SIZE PATH_MAX
#  else
#    define SLAVE_SIZE 1024
#  endif
  {
      static char slave[SLAVE_SIZE];
#  undef SLAVE_SIZE
      if (openpty(&mfd, sfdp, slave, NULL, NULL) == 0) {
	  *ptyslave = slave;
	  return mfd;
      }
  }
#elif !defined(HAVE_WORKING_POSIX_OPENPT)
  char *major, *minor;
  static char majorchars[] = "pqrstuvwxyzabcdePQRSTUVWXYZABCDE";
  static char minorchars[] = "0123456789abcdefghijklmnopqrstuv"
			     "wxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_+";
  {
    static char ptyname[] = "/dev/pty/mX";
    for (minor = minorchars; *minor; minor++) {
      ptyname[10] = *minor;
      if ((mfd = sf_open(ptyname, O_RDWR, 0)) >= 0) {
	ptyname[9] = 's';
	*ptyslave = ptyname;
	return mfd;
      }
    }
  }
  {
    static char ptyname[] = "/dev/ptym/ptyXY";
    static char ttyname[] = "/dev/pty/ttyXY";
    for (major = majorchars; *major; major++) {
      ptyname[13] = *major;
      for (minor = minorchars; *minor; minor++) {
	ptyname[14] = *minor;
	if ((mfd = sf_open(ptyname, O_RDWR, 0)) >= 0) {
	  ttyname[12] = *major;
	  ttyname[13] = *minor;
	  *ptyslave = ttyname;
	  return mfd;
	}
      }
    }
  }
  {
    static char ptyname[] = "/dev/ptyXY";
    for (major = majorchars; *major; major++) {
      ptyname[8] = *major;
      for (minor = minorchars; *minor; minor++) {
	ptyname[9] = *minor;
	if ((mfd = sf_open(ptyname, O_RDWR, 0)) >= 0) {
	  ptyname[5] = 't';
	  *ptyslave = ptyname;
	  return mfd;
	}
      }
    }
  }
#endif
  return -1;
}
static int open_pty_slave(char *name)
{
  int sfd;
  struct termios tty_rmode;
  if ((sfd = sf_open(name, O_RDWR, 0)) < 0) {
    return -1;
  }
#if defined(__sun) && defined(__SVR4)
  if ((ioctl(sfd, I_FIND, "ldterm")) < 0) {
    ERROR0(LOG_ERR, "Failed to find ldterm STREAMS module");
    return -1;
  }
  if (ioctl(sfd, I_PUSH, "ptem") < 0) {
    ERROR0(LOG_ERR, "Failed to push ptem STREAMS module");
    return -1;
  }
  if (ioctl(sfd, I_PUSH, "ldterm") < 0) {
    ERROR0(LOG_ERR, "Failed to push ldterm STREAMS module");
    return -1;
  }
  if (ioctl(sfd, I_PUSH, "ttcompat") < 0) {
    ERROR0(LOG_ERR, "Failed to push ttcompat STREAMS module");
    return -1;
  }
#endif
  if (getenv("RUN_ERL_DISABLE_FLOWCNTRL")) {
    if (tcgetattr(sfd, &tty_rmode) < 0) {
      fprintf(stderr, "Cannot get terminal's current mode\n");
      exit(-1);
    }
    tty_rmode.c_iflag &= ~IXOFF;
    if (tcsetattr(sfd, TCSANOW, &tty_rmode) < 0) {
      fprintf(stderr, "Cannot disable terminal's flow control on input\n");
      exit(-1);
    }
    tty_rmode.c_iflag &= ~IXON;
    if (tcsetattr(sfd, TCSANOW, &tty_rmode) < 0) {
      fprintf(stderr, "Cannot disable terminal's flow control on output\n");
      exit(-1);
    }
  }
#ifdef DEBUG
  if (tcgetattr(sfd, &tty_rmode) < 0) {
    fprintf(stderr, "Cannot get terminals current mode\n");
    exit(-1);
  }
  show_terminal_settings(&tty_rmode);
#endif
  return sfd;
}
static void exec_shell(char **argv)
{
  char *sh, **vp;
  int i;
  sh = "/bin/sh";
  if ((argv[0] = strrchr(sh, '/')) != NULL)
    argv[0]++;
  else
    argv[0] = sh;
  argv[1] = "-c";
  status("Args before exec of shell:\n");
  for (vp = argv, i = 0; *vp; vp++, i++)
    status("argv[%d] = %s\n", i, *vp);
  if (stdstatus) {
      fclose(stdstatus);
  }
  execv(sh, argv);
  if (run_daemon) {
      OPEN_SYSLOG();
  }
  ERRNO_ERR0(LOG_ERR,"Could not execv");
}
static void status(const char *format,...)
{
  va_list args;
  time_t now;
  if (stdstatus == NULL)
    stdstatus = fopen(statusfile, "w");
  if (stdstatus == NULL)
    return;
  now = time(NULL);
  fprintf(stdstatus, "run_erl [%d] %s", (int)getpid(), ctime(&now));
  va_start(args, format);
  vfprintf(stdstatus, format, args);
  va_end(args);
  fflush(stdstatus);
}
static void daemon_init(void)
{
    pid_t pid;
    int i, maxfd = HIGHEST_FILENO();
    if ((pid = fork()) != 0)
	exit(0);
#if defined(USE_SETPGRP_NOARGS)
    setpgrp();
#elif defined(USE_SETPGRP)
    setpgrp(0,getpid());
#else
    setsid();
#endif
    signal(SIGHUP, SIG_IGN);
    if ((pid = fork()) != 0)
	exit(0);
    for (i = 0; i < maxfd; ++i ) {
	sf_close(i);
    }
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);
    errno = 0;
    OPEN_SYSLOG();
    run_daemon = 1;
}
static void error_logf(int priority, int line, const char *format, ...)
{
    va_list args;
    va_start(args, format);
#ifdef HAVE_SYSLOG_H
    if (run_daemon) {
#ifdef HAVE_VSYSLOG
	vsyslog(priority,format,args);
#else
	va_list ap;
	char message[900];
	va_start (ap, format);
	vsnprintf(message, 900, format, ap);
	va_end(ap);
	syslog(priority, message);
#endif
    }
    else
#endif
    {
	time_t now = time(NULL);
	fprintf(stderr, "run_erl:%d [%d] %s", line, (int)getpid(), ctime(&now));
	vfprintf(stderr, format, args);
    }
    va_end(args);
}
static void usage(char *pname)
{
  fprintf(stderr, "Usage: %s (pipe_name|pipe_dir/) log_dir \"command [parameters ...]\"\n", pname);
  fprintf(stderr, "\nYou may also set the environment variables RUN_ERL_LOG_GENERATIONS\n");
  fprintf(stderr, "and RUN_ERL_LOG_MAXSIZE to the number of log files to use and the\n");
  fprintf(stderr, "size of the log file when to switch to the next log file\n");
}
static char *simple_basename(char *path)
{
    char *ptr;
    for (ptr = path; *ptr != '\0'; ++ptr) {
	if (*ptr == '/') {
	    path = ptr + 1;
	}
    }
    return path;
}
static void init_outbuf(void)
{
    outbuf_total = 1;
    outbuf_base = malloc(BUFSIZ);
    clear_outbuf();
}
static void clear_outbuf(void)
{
    outbuf_in = outbuf_out = outbuf_base;
}
static int outbuf_size(void)
{
    return outbuf_in - outbuf_out;
}
static char* outbuf_first(void)
{
    return outbuf_out;
}
static void outbuf_delete(int bytes)
{
    outbuf_out += bytes;
    if (outbuf_out >= outbuf_in) {
	outbuf_in = outbuf_out = outbuf_base;
    }
}
static void outbuf_append(const char* buf, int n)
{
    if (outbuf_base+outbuf_total < outbuf_in+n) {
	if (outbuf_out > outbuf_base) {
	    int size = outbuf_in - outbuf_out;
	    char* p;
	    outbuf_in -= outbuf_out - outbuf_base;
	    p = outbuf_base;
	    while (size-- > 0) {
		*p++ = *outbuf_out++;
	    }
	    outbuf_out = outbuf_base;
	}
	if (outbuf_base+outbuf_total < outbuf_in+n) {
	    int size = outbuf_in - outbuf_out;
	    outbuf_total = size+n;
	    outbuf_base = realloc(outbuf_base, outbuf_total);
	    outbuf_out = outbuf_base;
	    outbuf_in = outbuf_base + size;
	}
    }
    memcpy(outbuf_in, buf, n);
    outbuf_in += n;
}
static int write_all(int fd, const char* buf, int len)
{
    int left = len;
    int written;
    for (;;) {
	written = sf_write(fd,buf,left);
	if (written == left) {
	    return len;
	}
	if (written < 0) {
	    return -1;
	}
	left -= written;
	buf += written;
    }
}
static ssize_t sf_read(int fd, void *buffer, size_t len) {
    ssize_t n = 0;
    do { n = read(fd, buffer, len); } while (n < 0 && errno == EINTR);
    return n;
}
static ssize_t sf_write(int fd, const void *buffer, size_t len) {
    ssize_t n = 0;
    do { n = write(fd, buffer, len); } while (n < 0 && errno == EINTR);
    return n;
}
static int sf_open(const char *path, int type, mode_t mode) {
    int fd = 0;
    do { fd = open(path, type, mode); } while(fd < 0 && errno == EINTR);
    return fd;
}
static int sf_close(int fd) {
    return close(fd);
}
static int extract_ctrl_seq(char* buf, int len)
{
    static const char prefix[] = "\033_";
    static const char suffix[] = "\033\\";
    char* bufend = buf + len;
    char* start = buf;
    char* command;
    char* end;
    for (;;) {
	start = find_str(start, bufend-start, prefix);
	if (!start) break;
	command = start + strlen(prefix);
	end = find_str(command, bufend-command, suffix);
	if (end) {
	    unsigned col, row;
	    if (sscanf(command,"version=%u", &protocol_ver)==1) {
	    }
	    else if (sscanf(command,"winsize=%u,%u", &col, &row)==2) {
		set_window_size(col,row);
	    }
	    else {
		ERROR2(LOG_ERR, "Ignoring unknown ctrl command '%.*s'\n",
		       (int)(end-command), command);
	    }
	    end += strlen(suffix);
	    memmove(start, end, bufend-end);
	    bufend -= end - start;
	}
	else {
	    ERROR2(LOG_ERR, "Missing suffix in ctrl sequence '%.*s'\n",
		   (int)(bufend-start), start);
	    break;
	}
    }
    return bufend - buf;
}
static void set_window_size(unsigned col, unsigned row)
{
#ifdef TIOCSWINSZ
    struct winsize ws;
    ws.ws_col = col;
    ws.ws_row = row;
    if (ioctl(mfd, TIOCSWINSZ, &ws) < 0) {
	ERRNO_ERR0(LOG_ERR,"Failed to set window size");
    }
#endif
}
#ifdef DEBUG
#define S(x)  ((x) > 0 ? 1 : 0)
static void show_terminal_settings(struct termios *t)
{
  printf("c_iflag:\n");
  printf("Signal interrupt on break:   BRKINT  %d\n", S(t->c_iflag & BRKINT));
  printf("Map CR to NL on input:       ICRNL   %d\n", S(t->c_iflag & ICRNL));
  printf("Ignore break condition:      IGNBRK  %d\n", S(t->c_iflag & IGNBRK));
  printf("Ignore CR:                   IGNCR   %d\n", S(t->c_iflag & IGNCR));
  printf("Ignore char with par. err's: IGNPAR  %d\n", S(t->c_iflag & IGNPAR));
  printf("Map NL to CR on input:       INLCR   %d\n", S(t->c_iflag & INLCR));
  printf("Enable input parity check:   INPCK   %d\n", S(t->c_iflag & INPCK));
  printf("Strip character              ISTRIP  %d\n", S(t->c_iflag & ISTRIP));
  printf("Enable start/stop input ctrl IXOFF   %d\n", S(t->c_iflag & IXOFF));
  printf("ditto output ctrl            IXON    %d\n", S(t->c_iflag & IXON));
  printf("Mark parity errors           PARMRK  %d\n", S(t->c_iflag & PARMRK));
  printf("\n");
  printf("c_oflag:\n");
  printf("Perform output processing    OPOST   %d\n", S(t->c_oflag & OPOST));
  printf("\n");
  printf("c_cflag:\n");
  printf("Ignore modem status lines    CLOCAL  %d\n", S(t->c_cflag & CLOCAL));
  printf("\n");
  printf("c_local:\n");
  printf("Enable echo                  ECHO    %d\n", S(t->c_lflag & ECHO));
  printf("\n");
  printf("c_cc:\n");
  printf("c_cc[VEOF]                           %d\n", t->c_cc[VEOF]);
}
#endif