#include "etc_common.h"
#include <stddef.h>
#include <time.h>
#include <errno.h>
#if !defined(__WIN32__)
#  include <sys/types.h>
#  include <netinet/in.h>
#  include <sys/time.h>
#  include <unistd.h>
#  include <signal.h>
#  if defined(OS_MONOTONIC_TIME_USING_TIMES)
#    include <sys/times.h>
#    include <limits.h>
#  endif
#endif
#ifdef __clang_analyzer__
#  undef FD_ZERO
#  define FD_ZERO(FD_SET_PTR) memset(FD_SET_PTR, 0, sizeof(fd_set))
#endif
#define HEART_COMMAND_ENV          "HEART_COMMAND"
#define ERL_CRASH_DUMP_SECONDS_ENV "ERL_CRASH_DUMP_SECONDS"
#define HEART_KILL_SIGNAL          "HEART_KILL_SIGNAL"
#define HEART_NO_KILL              "HEART_NO_KILL"
#define MSG_HDR_SIZE         (2)
#define MSG_HDR_PLUS_OP_SIZE (3)
#define MSG_BODY_SIZE        (2048)
#define MSG_TOTAL_SIZE       (2050)
unsigned char cmd[MSG_BODY_SIZE];
struct msg {
  unsigned short len;
  unsigned char op;
  unsigned char fill[MSG_BODY_SIZE];
};
#define  HEART_ACK       (1)
#define  HEART_BEAT      (2)
#define  SHUT_DOWN       (3)
#define  SET_CMD         (4)
#define  CLEAR_CMD       (5)
#define  GET_CMD         (6)
#define  HEART_CMD       (7)
#define  PREPARING_CRASH (8)
#define  SELECT_TIMEOUT               5
int heart_beat_timeout = 60;
unsigned long heart_beat_kill_pid = 0;
#define  R_TIMEOUT          (1)
#define  R_CLOSED           (2)
#define  R_ERROR            (3)
#define  R_SHUT_DOWN        (4)
#define  R_CRASHING         (5)
#define  NULLFDS  ((fd_set *) NULL)
#define  NULLTV   ((struct timeval *) NULL)
static int message_loop(int, int);
static void do_terminate(int, int);
static int notify_ack(int);
static int heart_cmd_reply(int, char *);
static int write_message(int, struct msg *);
static int read_message(int, struct msg *);
static int read_skip(int, char *, int, int);
static int read_fill(int, char *, int);
static void print_error(const char *,...);
static void debugf(const char *,...);
static void init_timestamp(void);
static time_t timestamp(time_t *);
static int  wait_until_close_write_or_env_tmo(int);
#ifdef __WIN32__
static BOOL enable_privilege(void);
static BOOL do_shutdown(int);
static void print_last_error(void);
static HANDLE start_reader_thread(void);
static DWORD WINAPI reader(LPVOID);
#define read _read
#define write _write
#endif
static char program_name[256];
static int erlin_fd = 0, erlout_fd = 1;
static int debug_on = 0;
#ifdef __WIN32__
static HANDLE hreader_thread;
static HANDLE hevent_dataready;
static struct msg m, *mp = &m;
static int   tlen;
static FILE* conh;
#endif
static int
is_env_set(char *key)
{
#ifdef __WIN32__
    char buf[1];
    DWORD sz = (DWORD) sizeof(buf);
    SetLastError(0);
    sz = GetEnvironmentVariable((LPCTSTR) key, (LPTSTR) buf, sz);
    return sz || GetLastError() != ERROR_ENVVAR_NOT_FOUND;
#else
    return getenv(key) != NULL;
#endif
}
static char *
get_env(char *key)
{
#ifdef __WIN32__
    DWORD size = 32;
    char  *value=NULL;
    wchar_t *wcvalue = NULL;
    wchar_t wckey[256];
    int len;
    MultiByteToWideChar(CP_UTF8, 0, key, -1, wckey, 256);
    while (1) {
	DWORD nsz;
	if (wcvalue)
	    free(wcvalue);
	wcvalue = malloc(size*sizeof(wchar_t));
	if (!wcvalue) {
	    print_error("Failed to allocate memory. Terminating...");
	    exit(1);
	}
	SetLastError(0);
	nsz = GetEnvironmentVariableW(wckey, wcvalue, size);
	if (nsz == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
	    free(wcvalue);
	    return NULL;
	}
	if (nsz <= size) {
	    len = WideCharToMultiByte(CP_UTF8, 0, wcvalue, -1, NULL, 0, NULL, NULL);
	    value = malloc(len*sizeof(char));
	    if (!value) {
		print_error("Failed to allocate memory. Terminating...");
		exit(1);
	    }
	    WideCharToMultiByte(CP_UTF8, 0, wcvalue, -1, value, len, NULL, NULL);
	    free(wcvalue);
	    return value;
	}
	size = nsz;
    }
#else
    return getenv(key);
#endif
}
static void
free_env_val(char *value)
{
#ifdef __WIN32__
    if (value)
	free(value);
#endif
}
static void get_arguments(int argc, char** argv) {
    int i = 1;
    int h = -1;
    unsigned long p = 0;
    while (i < argc) {
	switch (argv[i][0]) {
	case '-':
	    switch (argv[i][1]) {
	    case 'h':
		if (strcmp(argv[i], "-ht") == 0)
		    if (sscanf(argv[i+1],"%i",&h) ==1)
			if ((h > 10) && (h <= 65535)) {
			    heart_beat_timeout = h;
			    fprintf(stderr,"heart_beat_timeout = %d\n",h);
			    i++;
			}
		break;
	    case 'p':
		if (strcmp(argv[i], "-pid") == 0)
		    if (sscanf(argv[i+1],"%lu",&p) ==1){
			heart_beat_kill_pid = p;
			fprintf(stderr,"heart_beat_kill_pid = %lu\n",p);
			i++;
		    }
		break;
#ifdef __WIN32__
	    case 's':
		if (strcmp(argv[i], "-shutdown") == 0){
		    do_shutdown(1);
		    exit(0);
		}
		break;
#endif
	    default:
		;
	    }
	    break;
	default:
	    ;
	}
	i++;
    }
    debugf("arguments -ht %d -pid %lu\n",h,p);
}
int main(int argc, char **argv) {
    if (is_env_set("HEART_DEBUG")) {
	fprintf(stderr, "heart: debug is ON!\r\n");
	debug_on = 1;
    }
    get_arguments(argc,argv);
#ifdef __WIN32__
    if (debug_on) {
	if(!is_env_set("ERLSRV_SERVICE_NAME")) {
	    erlin_fd = _dup(0);
	    erlout_fd = _dup(1);
	    AllocConsole();
	    conh = freopen("CONOUT$","w",stderr);
	    if (conh != NULL)
		fprintf(conh,"console allocated\n");
	}
	debugf("stderr\n");
    }
    _setmode(erlin_fd,_O_BINARY);
    _setmode(erlout_fd,_O_BINARY);
#endif
    strncpy(program_name, argv[0], sizeof(program_name));
    program_name[sizeof(program_name)-1] = '\0';
    notify_ack(erlout_fd);
    cmd[0] = '\0';
    do_terminate(erlin_fd,message_loop(erlin_fd,erlout_fd));
    return 0;
}
static int
message_loop(int erlin_fd, int erlout_fd)
{
  int   i;
  time_t now, last_received;
#ifdef __WIN32__
  DWORD wresult;
#else
  fd_set read_fds;
  int   max_fd;
  struct timeval timeout;
  int   tlen;
  struct msg m, *mp = &m;
#endif
  init_timestamp();
  timestamp(&now);
  last_received = now;
#ifdef __WIN32__
  hevent_dataready = CreateEvent(NULL,FALSE,FALSE,NULL);
  hreader_thread = start_reader_thread();
#else
  max_fd = erlin_fd;
#endif
  while (1) {
#ifdef __WIN32__
	wresult = WaitForSingleObject(hevent_dataready,SELECT_TIMEOUT*1000+ 2);
	if (wresult == WAIT_FAILED) {
		print_last_error();
		return R_ERROR;
	}
	if (wresult == WAIT_TIMEOUT) {
		debugf("wait timed out\n");
		i = 0;
	} else {
		debugf("wait ok\n");
		i = 1;
	}
#else
    FD_ZERO(&read_fds);
    FD_SET(erlin_fd, &read_fds);
    timeout.tv_sec = SELECT_TIMEOUT;
    timeout.tv_usec = 0;
    if ((i = select(max_fd + 1, &read_fds, NULLFDS, NULLFDS, &timeout)) < 0) {
      print_error("error in select.");
      return R_ERROR;
    }
#endif
    timestamp(&now);
    if (now > last_received + heart_beat_timeout) {
	print_error("heart-beat time-out, no activity for %lu seconds",
		    (unsigned long) (now - last_received));
		return R_TIMEOUT;
    }
    if (i == 0) {
      continue;
    }
#ifdef __WIN32__
	if (wresult == WAIT_OBJECT_0) {
		if (tlen < 0) {
#else
    if (FD_ISSET(erlin_fd, &read_fds)) {
		if ((tlen = read_message(erlin_fd, mp)) < 0) {
#endif
			print_error("error in read_message.");
			return R_ERROR;
		}
		if ((tlen > MSG_HDR_SIZE) && (tlen <= MSG_TOTAL_SIZE)) {
			switch (mp->op) {
			case HEART_BEAT:
				timestamp(&last_received);
				break;
			case SHUT_DOWN:
				return R_SHUT_DOWN;
			case SET_CMD:
			        memcpy(&cmd, &(mp->fill[0]),
				       tlen-MSG_HDR_PLUS_OP_SIZE);
			        cmd[tlen-MSG_HDR_PLUS_OP_SIZE] = '\0';
			        notify_ack(erlout_fd);
			        break;
			case CLEAR_CMD:
				cmd[0] = '\0';
				notify_ack(erlout_fd);
				break;
			case GET_CMD:
			        {
				    char *env = NULL;
				    char *command
					= (cmd[0]
					   ? (char *)cmd
					   : (env = get_env(HEART_COMMAND_ENV)));
				    if (!command) command = "";
				    heart_cmd_reply(erlout_fd, command);
				    free_env_val(env);
				}
			        break;
			case PREPARING_CRASH:
				print_error("Erlang is crashing .. (waiting for crash dump file)");
				return R_CRASHING;
			default:
				break;
			}
		} else if (tlen == 0) {
		print_error("Erlang has closed.");
		return R_CLOSED;
    }
    }
  }
}
#if defined(__WIN32__)
static void
kill_old_erlang(int reason){
    HANDLE erlh;
    DWORD exit_code;
    char* envvar = NULL;
    envvar = get_env(HEART_NO_KILL);
    if (envvar && strcmp(envvar, "TRUE") == 0)
      return;
    if(heart_beat_kill_pid != 0){
	if((erlh = OpenProcess(PROCESS_TERMINATE |
			       SYNCHRONIZE |
			       PROCESS_QUERY_INFORMATION ,
			       FALSE,
			       (DWORD) heart_beat_kill_pid)) == NULL){
	    return;
	}
	if(!TerminateProcess(erlh, 1)){
	    CloseHandle(erlh);
	    return;
	}
	if(WaitForSingleObject(erlh,5000) != WAIT_OBJECT_0){
	    print_error("Old process did not die, "
			"WaitForSingleObject timed out.");
	    CloseHandle(erlh);
	    return;
	}
	if(!GetExitCodeProcess(erlh, &exit_code)){
	    print_error("Old process did not die, "
			"GetExitCodeProcess failed.");
	}
	CloseHandle(erlh);
    }
}
#else
static void
kill_old_erlang(int reason)
{
    pid_t pid;
    int i, res;
    int sig = SIGKILL;
    char *envvar = NULL;
    envvar = get_env(HEART_NO_KILL);
    if (envvar && strcmp(envvar, "TRUE") == 0)
      return;
    if(heart_beat_kill_pid != 0){
        pid = (pid_t) heart_beat_kill_pid;
        if (reason == R_CLOSED) {
            print_error("Wait 5 seconds for Erlang to terminate nicely");
            for (i=0; i < 5; ++i) {
                res = kill(pid, 0);
                if (res < 0 && errno == ESRCH)
                    return;
                sleep(1);
            }
            print_error("Erlang still alive, kill it");
        }
        envvar = get_env(HEART_KILL_SIGNAL);
        if (envvar && strcmp(envvar, "SIGABRT") == 0) {
            print_error("kill signal SIGABRT requested");
            sig = SIGABRT;
        }
	res = kill(pid,sig);
	for(i=0; i < 5 && res == 0; ++i){
	    sleep(1);
	    res = kill(pid,sig);
	}
	if(errno != ESRCH){
	    print_error("Unable to kill old process, "
			"kill failed (tried multiple times).");
	}
    }
}
#endif
#ifdef __WIN32__
void win_system(char *command)
{
    char *comspec;
    char * cmdbuff;
    char * extra = " /C ";
    wchar_t *wccmdbuff;
    char *env;
    STARTUPINFOW start;
    SECURITY_ATTRIBUTES attr;
    PROCESS_INFORMATION info;
    int len;
    if (!debug_on) {
	len = MultiByteToWideChar(CP_UTF8, 0, command, -1, NULL, 0);
	wccmdbuff = malloc(len*sizeof(wchar_t));
	if (!wccmdbuff) {
	    print_error("Failed to allocate memory. Terminating...");
	    exit(1);
	}
	MultiByteToWideChar(CP_UTF8, 0, command, -1, wccmdbuff, len);
	_wsystem(wccmdbuff);
	return;
    }
    comspec = env = get_env("COMSPEC");
    if (!comspec)
	comspec = "CMD.EXE";
    cmdbuff = malloc(strlen(command) + strlen(comspec) + strlen(extra) + 1);
    if (!cmdbuff) {
	print_error("Failed to allocate memory. Terminating...");
	exit(1);
    }
    strcpy(cmdbuff, comspec);
    strcat(cmdbuff, extra);
    strcat(cmdbuff, command);
    free_env_val(env);
    debugf("running \"%s\"\r\n", cmdbuff);
    memset (&start, 0, sizeof (start));
    start.cb = sizeof (start);
    start.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    start.wShowWindow = SW_HIDE;
    start.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    start.hStdOutput = GetStdHandle(STD_ERROR_HANDLE);
    start.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    attr.nLength = sizeof(attr);
    attr.lpSecurityDescriptor = NULL;
    attr.bInheritHandle = TRUE;
    fflush(stderr);
    len = MultiByteToWideChar(CP_UTF8, 0, cmdbuff, -1, NULL, 0);
    wccmdbuff = malloc(len*sizeof(wchar_t));
    if (!wccmdbuff) {
	print_error("Failed to allocate memory. Terminating...");
	exit(1);
    }
    MultiByteToWideChar(CP_UTF8, 0, cmdbuff, -1, wccmdbuff, len);
    if (!CreateProcessW(NULL,
			wccmdbuff,
			&attr,
			NULL,
			TRUE,
			0,
			NULL,
			NULL,
			&start,
			&info)) {
	debugf("Could not create process for the command %s.\r\n", cmdbuff);
    }
    WaitForSingleObject(info.hProcess,INFINITE);
    free(cmdbuff);
    free(wccmdbuff);
}
#endif
static void
do_terminate(int erlin_fd, int reason) {
  int ret = 0, tmo=0;
  char *tmo_env;
  switch (reason) {
  case R_SHUT_DOWN:
    break;
  case R_CRASHING:
    if (is_env_set(ERL_CRASH_DUMP_SECONDS_ENV)) {
	tmo_env = get_env(ERL_CRASH_DUMP_SECONDS_ENV);
	tmo = atoi(tmo_env);
	print_error("Waiting for dump - timeout set to %d seconds.", tmo);
	wait_until_close_write_or_env_tmo(tmo);
	free_env_val(tmo_env);
    }
  case R_TIMEOUT:
  case R_CLOSED:
  case R_ERROR:
  default:
    {
#if defined(__WIN32__)
	if(!cmd[0]) {
	    char *command = get_env(HEART_COMMAND_ENV);
	    if(!command)
		print_error("Would reboot. Terminating.");
	    else {
		kill_old_erlang(reason);
		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
		win_system(command);
		print_error("Executed \"%s\". Terminating.",command);
	    }
	    free_env_val(command);
	} else {
	    kill_old_erlang(reason);
	    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
	    win_system(&cmd[0]);
	    print_error("Executed \"%s\". Terminating.",cmd);
	}
#else
	if(!cmd[0]) {
	    char *command = get_env(HEART_COMMAND_ENV);
	    if(!command)
		print_error("Would reboot. Terminating.");
	    else {
		kill_old_erlang(reason);
		ret = system(command);
		print_error("Executed \"%s\" -> %d. Terminating.",command, ret);
	    }
	    free_env_val(command);
	} else {
	    kill_old_erlang(reason);
	    ret = system((char*)&cmd[0]);
	    print_error("Executed \"%s\" -> %d. Terminating.",cmd, ret);
	}
#endif
    }
    break;
  }
}
int wait_until_close_write_or_env_tmo(int tmo) {
    int i = 0;
#ifdef __WIN32__
    DWORD wresult;
    DWORD wtmo = INFINITE;
    if (tmo >= 0) {
	wtmo = tmo*1000 + 2;
    }
    wresult = WaitForSingleObject(hevent_dataready, wtmo);
    if (wresult == WAIT_FAILED) {
	print_last_error();
	return -1;
    }
    if (wresult == WAIT_TIMEOUT) {
	debugf("wait timed out\n");
	i = 0;
    } else {
	debugf("wait ok\n");
	i = 1;
    }
#else
    fd_set read_fds;
    int   max_fd;
    struct timeval timeout;
    struct timeval *tptr = NULL;
    max_fd = erlin_fd;
    if (tmo >= 0) {
	timeout.tv_sec  = tmo;
	timeout.tv_usec = 0;
	tptr = &timeout;
    }
    FD_ZERO(&read_fds);
    FD_SET(erlin_fd, &read_fds);
    if ((i = select(max_fd + 1, &read_fds, NULLFDS, NULLFDS, tptr)) < 0) {
	print_error("error in select.");
	return -1;
    }
#endif
    return i;
}
static int
notify_ack(int fd)
{
  struct msg m;
  m.op = HEART_ACK;
  m.len = htons(1);
  return write_message(fd, &m);
}
static int
heart_cmd_reply(int fd, char *s)
{
  struct msg m;
  int len = strlen(s);
  if (len >= sizeof(m.fill))
      return -1;
  m.op = HEART_CMD;
  m.len = htons(len + 1);
  strcpy((char*)m.fill, s);
  return write_message(fd, &m);
}
static int
write_message(int fd, struct msg *mp)
{
  int len = ntohs(mp->len);
  if ((len == 0) || (len > MSG_BODY_SIZE)) {
    return MSG_HDR_SIZE;
  }
  if (write(fd, (char *) mp, len + MSG_HDR_SIZE) != len + MSG_HDR_SIZE) {
    return -1;
  }
  return len + MSG_HDR_SIZE;
}
static int
read_message(int fd, struct msg *mp)
{
  int   rlen, i;
  unsigned char* tmp;
  if ((i = read_fill(fd, (char *) mp, MSG_HDR_SIZE)) != MSG_HDR_SIZE) {
    return i;
  }
  tmp = (unsigned char*) &(mp->len);
  rlen = (*tmp * 256) + *(tmp+1);
  if (rlen == 0) {
    return MSG_HDR_SIZE;
  }
  if (rlen > MSG_BODY_SIZE) {
    if ((i = read_skip(fd, (((char *) mp) + MSG_HDR_SIZE),
		       MSG_BODY_SIZE, rlen)) != rlen) {
      return i;
    } else {
      return rlen + MSG_HDR_SIZE;
    }
  }
  if ((i = read_fill(fd, ((char *) mp + MSG_HDR_SIZE), rlen)) != rlen) {
    return i;
  }
  return rlen + MSG_HDR_SIZE;
}
static int
read_fill(int fd, char *buf, int len)
{
  int   i, got = 0;
  do {
    if ((i = read(fd, buf + got, len - got)) <= 0) {
      return i;
    }
    got += i;
  } while (got < len);
  return len;
}
static int
read_skip(int fd, char *buf, int maxlen, int len)
{
  int   i, got = 0;
  char  c;
  if ((i = read_fill(fd, buf, maxlen)) <= 0) {
    return i;
  }
  do {
    if ((i = read(fd, &c, 1)) <= 0) {
      return i;
    }
    got += i;
  } while (got < len - maxlen);
  return len;
}
static void
print_error(const char *format,...)
{
  va_list args;
  time_t now;
  char *timestr;
  va_start(args, format);
  time(&now);
  timestr = ctime(&now);
  fprintf(stderr, "%s: %.*s: ", program_name, (int) strlen(timestr)-1, timestr);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, "\r\n");
}
static void
debugf(const char *format,...)
{
  va_list args;
  if (debug_on) {
      va_start(args, format);
      fprintf(stderr, "Heart: ");
      vfprintf(stderr, format, args);
      va_end(args);
      fprintf(stderr, "\r\n");
  }
}
#ifdef __WIN32__
void print_last_error() {
	LPTSTR lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
	);
	fprintf(stderr,"GetLastError:%s\n",lpMsgBuf);
	LocalFree( lpMsgBuf );
}
static BOOL enable_privilege() {
	HANDLE ProcessHandle;
	DWORD DesiredAccess = TOKEN_ADJUST_PRIVILEGES;
	HANDLE TokenHandle;
	TOKEN_PRIVILEGES Tpriv;
	LUID luid;
	ProcessHandle = GetCurrentProcess();
	OpenProcessToken(ProcessHandle, DesiredAccess, &TokenHandle);
	LookupPrivilegeValue(0,SE_SHUTDOWN_NAME,&luid);
	Tpriv.PrivilegeCount = 1;
	Tpriv.Privileges[0].Luid = luid;
	Tpriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	return AdjustTokenPrivileges(TokenHandle,FALSE,&Tpriv,0,0,0);
}
static BOOL do_shutdown(int really_shutdown) {
    enable_privilege();
    if (really_shutdown) {
	if (InitiateSystemShutdown(NULL,"shutdown by HEART",10,TRUE,TRUE))
	    return TRUE;
    } else if (InitiateSystemShutdown(NULL,
				      "shutdown by HEART\n"
				      "will be interrupted",
				      30,TRUE,TRUE)) {
	AbortSystemShutdown(NULL);
	return TRUE;
    }
    return FALSE;
}
DWORD WINAPI reader(LPVOID lpvParam) {
	while (1) {
		debugf("reader is reading\n");
		tlen = read_message(erlin_fd, mp);
		debugf("reader setting event\n");
		SetEvent(hevent_dataready);
		if(tlen == 0)
		    break;
	}
	return 0;
}
HANDLE start_reader_thread(void) {
	DWORD tid;
	HANDLE thandle;
	if ((thandle = (HANDLE)
	     _beginthreadex(NULL,0,reader,NULL,0,&tid)) == NULL) {
		print_last_error();
		exit(1);
	}
	return thandle;
}
#endif
#if defined(__WIN32__)
#  define TICK_MASK 0x7FFFFFFFUL
void init_timestamp(void)
{
}
time_t timestamp(time_t *res)
{
    static time_t extra = 0;
    static unsigned last_ticks = 0;
    unsigned this_ticks;
    time_t r;
    this_ticks = GetTickCount() & TICK_MASK;
    if (this_ticks < last_ticks) {
	extra += (time_t) ((TICK_MASK + 1) / 1000);
    }
    last_ticks = this_ticks;
    r = ((time_t) (this_ticks / 1000)) + extra;
    if (res != NULL)
	*res = r;
    return r;
}
#elif defined(OS_MONOTONIC_TIME_USING_GETHRTIME) || defined(OS_MONOTONIC_TIME_USING_CLOCK_GETTIME)
#if defined(OS_MONOTONIC_TIME_USING_CLOCK_GETTIME)
typedef long long SysHrTime;
SysHrTime sys_gethrtime(void);
SysHrTime sys_gethrtime(void)
{
    struct timespec ts;
    long long result;
    if (clock_gettime(MONOTONIC_CLOCK_ID,&ts) != 0) {
	print_error("Fatal, could not get clock_monotonic value, terminating! "
		    "errno = %d\n", errno);
	exit(1);
    }
    result = ((long long) ts.tv_sec) * 1000000000LL +
	((long long) ts.tv_nsec);
    return (SysHrTime) result;
}
#else
typedef hrtime_t SysHrTime;
#define sys_gethrtime() gethrtime()
#endif
void init_timestamp(void)
{
}
time_t timestamp(time_t *res)
{
    SysHrTime ht = sys_gethrtime();
    time_t r = (time_t) (ht / 1000000000);
    if (res != NULL)
	*res = r;
    return r;
}
#elif defined(OS_MONOTONIC_TIME_USING_TIMES)
#  ifdef NO_SYSCONF
#    include <sys/param.h>
#    define TICKS_PER_SEC()	HZ
#  else
#    define TICKS_PER_SEC()	sysconf(_SC_CLK_TCK)
#  endif
#  define TICK_MASK 0x7FFFFFFFUL
static unsigned tps;
void init_timestamp(void)
{
    tps = TICKS_PER_SEC();
}
time_t timestamp(time_t *res)
{
    static time_t extra = 0;
    static clock_t last_ticks = 0;
    clock_t this_ticks;
    struct tms dummy;
    time_t r;
    this_ticks = (times(&dummy) & TICK_MASK);
    if (this_ticks < last_ticks) {
	extra += (time_t) ((TICK_MASK + 1) / tps);
    }
    last_ticks = this_ticks;
    r = ((time_t) (this_ticks / tps)) + extra;
    if (res != NULL)
	*res = r;
    return r;
}
#else
void init_timestamp(void)
{
}
time_t timestamp(time_t *res)
{
    return time(res);
}
#endif