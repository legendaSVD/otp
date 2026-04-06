#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "epmd.h"
#include "epmd_int.h"
#include "erl_printf.h"
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#include <time.h>
static void usage(EpmdVars *);
static void run_daemon(EpmdVars*);
static char* get_addresses(void);
static int get_port_no(void);
static int check_relaxed(void);
#ifdef __WIN32__
static int has_console(void);
#endif
#ifdef DONT_USE_MAIN
static int epmd_main(int, char **, int);
#define MAX_DEBUG 10
int epmd_dbg(int level,int port)
{
  char* argv[MAX_DEBUG+4];
  char  ibuff[100];
  int   argc = 0;
  argv[argc++] = "epmd";
  if(level > MAX_DEBUG)
    level = MAX_DEBUG;
  for(;level;--level)
    argv[argc++] = "-d";
  if(port)
    {
      argv[argc++] = "-port";
      erts_snprintf(ibuff, sizeof(ibuff), "%d",port);
      argv[argc++] = ibuff;
    }
  argv[argc] = NULL;
  return epmd(argc,argv);
}
static char *mystrdup(char *s)
{
    char *r = malloc(strlen(s)+1);
    strcpy(r,s);
    return r;
}
int epmd(int argc, char **argv)
{
  return epmd_main(argc,argv,0);
}
static int epmd_main(int argc, char** argv, int free_argv)
#else
int main(int argc, char** argv)
#endif
{
    EpmdVars g_empd_vars;
    EpmdVars *g = &g_empd_vars;
    int i;
#ifdef __WIN32__
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(1, 1);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
        epmd_cleanup_exit(g,1);
    if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion ) != 1) {
        WSACleanup();
    	epmd_cleanup_exit(g,1);
    }
#endif
#ifdef DONT_USE_MAIN
    if(free_argv)
	g->argv = argv;
    else
	g->argv = NULL;
#else
    g->argv = NULL;
#endif
    g->addresses      = get_addresses();
    g->port           = get_port_no();
    g->debug          = 0;
    g->silent         = 0;
    g->is_daemon      = 0;
    g->brutal_kill    = check_relaxed();
    g->packet_timeout = CLOSE_TIMEOUT;
    g->delay_accept   = 0;
    g->delay_write    = 0;
    g->progname       = argv[0];
    g->conn           = NULL;
    g->nodes.reg = g->nodes.unreg = g->nodes.unreg_tail = NULL;
    g->nodes.unreg_count = 0;
    g->active_conn    = 0;
#ifdef HAVE_SYSTEMD_DAEMON
    g->is_systemd     = 0;
#endif
    for (i = 0; i < MAX_LISTEN_SOCKETS; i++)
	g->listenfd[i] = -1;
    argc--;
    argv++;
    while (argc > 0) {
	if ((strcmp(argv[0], "-debug")==0) ||
	    (strcmp(argv[0], "-d")==0)) {
	    g->debug += 1;
	    argv++; argc--;
	} else if (strcmp(argv[0], "-packet_timeout") == 0) {
	    if ((argc == 1) ||
		((g->packet_timeout = atoi(argv[1])) == 0))
		usage(g);
	    argv += 2; argc -= 2;
	} else if (strcmp(argv[0], "-delay_accept") == 0) {
	    if ((argc == 1) ||
		((g->delay_accept = atoi(argv[1])) == 0))
		usage(g);
	    argv += 2; argc -= 2;
	} else if (strcmp(argv[0], "-delay_write") == 0) {
	    if ((argc == 1) ||
		((g->delay_write = atoi(argv[1])) == 0))
		usage(g);
	    argv += 2; argc -= 2;
	} else if (strcmp(argv[0], "-daemon") == 0) {
	    g->is_daemon = 1;
	    argv++; argc--;
	} else if (strcmp(argv[0], "-relaxed_command_check") == 0) {
	    g->brutal_kill = 1;
	    argv++; argc--;
	} else if (strcmp(argv[0], "-kill") == 0) {
	    if (argc == 1)
		kill_epmd(g);
	    else
		usage(g);
	    epmd_cleanup_exit(g,0);
	} else if (strcmp(argv[0], "-address") == 0) {
	    if (argc == 1)
	      usage(g);
	    g->addresses = argv[1];
	    argv += 2; argc -= 2;
	} else if (strcmp(argv[0], "-port") == 0) {
	    if ((argc == 1) ||
		((g->port = atoi(argv[1])) == 0))
	      usage(g);
	    argv += 2; argc -= 2;
	} else if (strcmp(argv[0], "-names") == 0) {
	    if (argc == 1)
		epmd_call(g, EPMD_NAMES_REQ);
	    else
		usage(g);
	    epmd_cleanup_exit(g,0);
	} else if (strcmp(argv[0], "-started") == 0) {
	    g->silent = 1;
	    if (argc == 1)
		epmd_call(g, EPMD_NAMES_REQ);
	    else
		usage(g);
	    epmd_cleanup_exit(g,0);
	} else if (strcmp(argv[0], "-dump") == 0) {
	    if (argc == 1)
		epmd_call(g, EPMD_DUMP_REQ);
	    else
		usage(g);
	    epmd_cleanup_exit(g,0);
	} else if (strcmp(argv[0], "-stop") == 0) {
	    if (argc == 2)
		stop_cli(g, argv[1]);
	    else
		usage(g);
	    epmd_cleanup_exit(g,0);
#ifdef HAVE_SYSTEMD_DAEMON
	} else if (strcmp(argv[0], "-systemd") == 0) {
            g->is_systemd = 1;
            argv++; argc--;
#endif
	} else
	    usage(g);
    }
    dbg_printf(g,1,"epmd running - daemon = %d",g->is_daemon);
#ifndef NO_SYSCONF
    if ((g->max_conn = sysconf(_SC_OPEN_MAX)) <= 0)
#endif
      g->max_conn = MAX_FILES;
    if (g->max_conn > FD_SETSIZE) {
      g->max_conn = FD_SETSIZE;
    }
    if (g->is_daemon)  {
	run_daemon(g);
    } else {
	run(g);
    }
    return 0;
}
#ifndef NO_DAEMON
static void run_daemon(EpmdVars *g)
{
    register int child_pid, fd;
    dbg_tty_printf(g,2,"fork a daemon");
    if (( child_pid = fork()) < 0)
      {
#ifdef HAVE_SYSLOG_H
	syslog(LOG_ERR,"erlang mapper daemon can't fork %m");
#endif
	epmd_cleanup_exit(g,1);
      }
    else if (child_pid > 0)
      {
	dbg_tty_printf(g,2,"daemon child is %d",child_pid);
	epmd_cleanup_exit(g,0);
      }
    if (setsid() < 0)
      {
	dbg_perror(g,"epmd: Can't setsid()");
	epmd_cleanup_exit(g,1);
      }
    signal(SIGHUP, SIG_IGN);
    if ((child_pid = fork()) < 0)
      {
#ifdef HAVE_SYSLOG_H
	syslog(LOG_ERR,"erlang mapper daemon can't fork 2'nd time %m");
#endif
	epmd_cleanup_exit(g,1);
      }
    else if (child_pid > 0)
      {
	dbg_tty_printf(g,2,"daemon 2'nd child is %d",child_pid);
	epmd_cleanup_exit(g,0);
      }
    if (chdir("/") < 0)
      {
	dbg_perror(g,"epmd: chdir() failed");
	epmd_cleanup_exit(g,1);
      }
    umask(0);
    for (fd = 0; fd < g->max_conn ; fd++)
        close(fd);
    closelog();
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);
    errno = 0;
    run(g);
}
#endif
#ifdef __WIN32__
static int has_console(void)
{
    HANDLE handle = CreateFile("CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE,
			       NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
	return 0;
    } else {
        CloseHandle(handle);
	return 1;
    }
}
static void run_daemon(EpmdVars *g)
{
    if (has_console()) {
	if (spawnvp(_P_DETACH, __argv[0], __argv) == -1) {
	    fprintf(stderr, "Failed to spawn detached epmd\n");
	    exit(1);
	}
	exit(0);
    }
    close(0);
    close(1);
    close(2);
    open("nul", O_RDONLY);
    open("nul", O_WRONLY);
    open("nul", O_WRONLY);
    run(g);
}
#endif
static void usage(EpmdVars *g)
{
    fprintf(stderr, "usage: epmd [-d|-debug] [DbgExtra...] [-address List]\n");
    fprintf(stderr, "            [-port No] [-daemon] [-relaxed_command_check]\n");
    fprintf(stderr, "       epmd [-d|-debug] [-port No] [-names|-kill|-stop name]\n\n");
    fprintf(stderr, "See the Erlang epmd manual page for info about the usage.\n\n");
    fprintf(stderr, "Regular options\n");
    fprintf(stderr, "    -address List\n");
    fprintf(stderr, "        Let epmd listen only on the comma-separated list of IP\n");
    fprintf(stderr, "        addresses (and on the loopback interface).\n");
    fprintf(stderr, "    -port No\n");
    fprintf(stderr, "        Let epmd listen to another port than default %d\n",
	    EPMD_PORT_NO);
    fprintf(stderr, "    -d\n");
    fprintf(stderr, "    -debug\n");
    fprintf(stderr, "        Enable debugging. This will give a log to\n");
    fprintf(stderr, "        the standard error stream. It will shorten\n");
    fprintf(stderr, "        the number of saved used node names to 5.\n\n");
    fprintf(stderr, "        If you give more than one debug flag you may\n");
    fprintf(stderr, "        get more debugging information.\n");
    fprintf(stderr, "    -daemon\n");
    fprintf(stderr, "        Start epmd detached (as a daemon)\n");
    fprintf(stderr, "    -relaxed_command_check\n");
    fprintf(stderr, "        Allow this instance of epmd to be killed with\n");
    fprintf(stderr, "        epmd -kill even if there "
	    "are registered nodes.\n");
    fprintf(stderr, "        Also allows forced unregister (epmd -stop).\n");
#ifdef HAVE_SYSTEMD_DAEMON
    fprintf(stderr, "    -systemd\n");
    fprintf(stderr, "        Wait for socket from systemd. The option makes sense\n");
    fprintf(stderr, "        when started from .socket unit.\n");
#endif
    fprintf(stderr, "\nDbgExtra options\n");
    fprintf(stderr, "    -packet_timeout Seconds\n");
    fprintf(stderr, "        Set the number of seconds a connection can be\n");
    fprintf(stderr, "        inactive before epmd times out and closes the\n");
    fprintf(stderr, "        connection (default 60).\n\n");
    fprintf(stderr, "    -delay_accept Seconds\n");
    fprintf(stderr, "        To simulate a busy server you can insert a\n");
    fprintf(stderr, "        delay between epmd gets notified about that\n");
    fprintf(stderr, "        a new connection is requested and when the\n");
    fprintf(stderr, "        connections gets accepted.\n\n");
    fprintf(stderr, "    -delay_write Seconds\n");
    fprintf(stderr, "        Also a simulation of a busy server. Inserts\n");
    fprintf(stderr, "        a delay before a reply is sent.\n");
    fprintf(stderr, "\nInteractive options\n");
    fprintf(stderr, "    -names\n");
    fprintf(stderr, "        List names registered with the currently "
	    "running epmd\n");
    fprintf(stderr, "    -kill\n");
    fprintf(stderr, "        Kill the currently running epmd\n");
    fprintf(stderr, "        (only allowed if -names show empty database or\n");
    fprintf(stderr, "        -relaxed_command_check was given when epmd was started).\n");
    fprintf(stderr, "    -stop Name\n");
    fprintf(stderr, "        Forcibly unregisters a name with epmd\n");
    fprintf(stderr, "        (only allowed if -relaxed_command_check was given when \n");
    fprintf(stderr, "        epmd was started).\n");
    epmd_cleanup_exit(g,1);
}
#define DEBUG_BUFFER_SIZE 2048
static void dbg_gen_printf(int onsyslog,int perr,int from_level,
			   EpmdVars *g,const char *format, va_list args)
{
  time_t now;
  char *timestr;
  char buf[DEBUG_BUFFER_SIZE];
  if (g->is_daemon)
    {
#ifdef HAVE_SYSLOG_H
      if (onsyslog)
	{
	  erts_vsnprintf(buf, DEBUG_BUFFER_SIZE, format, args);
	  syslog(LOG_ERR,"epmd: %s",buf);
	}
#endif
    }
  else
    {
      int len;
      time(&now);
      timestr = (char *)ctime(&now);
      erts_snprintf(buf, DEBUG_BUFFER_SIZE, "epmd: %.*s: ",
		    (int) strlen(timestr)-1, timestr);
      len = strlen(buf);
      erts_vsnprintf(buf + len, DEBUG_BUFFER_SIZE - len, format, args);
      if (perr != 0)
	fprintf(stderr,"%s: %s\r\n",buf,strerror(perr));
      else
	fprintf(stderr,"%s\r\n",buf);
    }
}
void dbg_perror(EpmdVars *g,const char *format,...)
{
  va_list args;
  va_start(args, format);
  dbg_gen_printf(1,errno,0,g,format,args);
  va_end(args);
}
void dbg_tty_printf(EpmdVars *g,int from_level,const char *format,...)
{
  if (g->debug >= from_level) {
    va_list args;
    va_start(args, format);
    dbg_gen_printf(0,0,from_level,g,format,args);
    va_end(args);
  }
}
void dbg_printf(EpmdVars *g,int from_level,const char *format,...)
{
  if (g->debug >= from_level) {
    va_list args;
    va_start(args, format);
    dbg_gen_printf(1,0,from_level,g,format,args);
    va_end(args);
  }
}
static void free_all_nodes(EpmdVars *g)
{
    Node *tmp;
    for(tmp=g->nodes.reg; tmp != NULL; tmp = g->nodes.reg){
	g->nodes.reg = tmp->next;
	free(tmp);
    }
    for(tmp=g->nodes.unreg; tmp != NULL; tmp = g->nodes.unreg){
	g->nodes.unreg = tmp->next;
	free(tmp);
    }
}
__decl_noreturn void __noreturn
epmd_cleanup_exit(EpmdVars *g, int exitval)
{
  int i;
  if(g->conn){
      for (i = 0; i < g->max_conn; i++)
	  if (g->conn[i].open == EPMD_TRUE)
	      epmd_conn_close(g,&g->conn[i]);
      free(g->conn);
  }
  for(i=0; i < MAX_LISTEN_SOCKETS; i++)
      if(g->listenfd[i] >= 0)
          close(g->listenfd[i]);
  free_all_nodes(g);
  if(g->argv){
      for(i=0; g->argv[i] != NULL; ++i)
	  free(g->argv[i]);
      free(g->argv);
  }
#ifdef HAVE_SYSTEMD_DAEMON
  if (g->is_systemd){
    sd_notifyf(0, "STATUS=Exited.\n"
               "ERRNO=%i", exitval);
  }
#endif
  exit(exitval);
}
static char* get_addresses(void)
{
    return getenv("ERL_EPMD_ADDRESS");
}
static int get_port_no(void)
{
    char* port_str = getenv("ERL_EPMD_PORT");
    return (port_str != NULL) ? atoi(port_str) : EPMD_PORT_NO;
}
static int check_relaxed(void)
{
    char* port_str = getenv("ERL_EPMD_RELAXED_COMMAND_CHECK");
    return (port_str != NULL) ? 1 : 0;
}