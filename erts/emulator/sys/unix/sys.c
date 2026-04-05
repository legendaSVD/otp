#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#ifdef ISC32
#define _POSIX_SOURCE
#define _XOPEN_SOURCE
#endif
#include <sys/times.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <termios.h>
#include <ctype.h>
#include <sys/utsname.h>
#include <sys/select.h>
#ifdef ISC32
#include <sys/bsdtypes.h>
#endif
#include <termios.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef ADDRESS_SANITIZER
#  include <sanitizer/asan_interface.h>
#endif
#define ERTS_WANT_BREAK_HANDLING
#define WANT_NONBLOCKING
#include "sys.h"
#include "erl_thr_progress.h"
#if defined(__APPLE__) && defined(__MACH__) && !defined(__DARWIN__)
#define __DARWIN__ 1
#endif
#include "erl_threads.h"
#include "erl_mseg.h"
#define MAX_VSIZE 16
#include "global.h"
#include "bif.h"
#include "erl_check_io.h"
#include "erl_cpu_topology.h"
#include "erl_osenv.h"
#include "erl_dyn_lock_check.h"
extern int  driver_interrupt(int, int);
extern void do_break(void);
extern void erl_sys_args(int*, char**);
extern void erts_sys_init_float(void);
#ifdef DEBUG
static int debug_log = 0;
#endif
static erts_atomic32_t have_prepared_crash_dump;
#define ERTS_PREPARED_CRASH_DUMP \
  ((int) erts_atomic32_xchg_nob(&have_prepared_crash_dump, 1))
erts_atomic_t sys_misc_mem_sz;
static void smp_sig_notify(int signum);
static int sig_notify_fds[2] = {-1, -1};
#ifdef ERTS_SYS_SUSPEND_SIGNAL
static int sig_suspend_fds[2] = {-1, -1};
#endif
jmp_buf erts_sys_sigsegv_jmp;
static int crashdump_companion_cube_fd = -1;
static int max_files = -1;
erts_atomic32_t erts_break_requested;
#define ERTS_SET_BREAK_REQUESTED \
  erts_atomic32_set_nob(&erts_break_requested, (erts_aint32_t) 1)
#define ERTS_UNSET_BREAK_REQUESTED \
  erts_atomic32_set_nob(&erts_break_requested, (erts_aint32_t) 0)
struct termios erl_sys_initial_tty_mode;
static int replace_intr = 0;
int using_oldshell = 1;
UWord sys_page_size;
UWord sys_large_page_size;
static UWord
get_page_size(void)
{
#if defined(_SC_PAGESIZE)
    return (UWord) sysconf(_SC_PAGESIZE);
#elif defined(HAVE_GETPAGESIZE)
    return (UWord) getpagesize();
#else
    return (UWord) 4*1024;
#endif
}
static UWord
get_large_page_size(void)
{
#ifdef HAVE_LINUX_THP
    FILE *fp;
    UWord result;
    int matched;
    fp = fopen("/sys/kernel/mm/transparent_hugepage/hpage_pmd_size", "r");
    if (fp == NULL)
        return 0;
    matched = fscanf(fp, "%lu", &result);
    fclose(fp);
    return matched == 1 ? result : 0;
#else
    return 0;
#endif
}
Uint
erts_sys_misc_mem_sz(void)
{
    Uint res = erts_check_io_size();
    res += erts_atomic_read_mb(&sys_misc_mem_sz);
    return res;
}
void sys_tty_reset(int exit_code)
{
  if (using_oldshell && !replace_intr) {
    SET_BLOCKING(0);
  }
  else if (isatty(0) && isatty(1)) {
    tcsetattr(0,TCSANOW,&erl_sys_initial_tty_mode);
  }
}
#ifdef __tile__
#include <malloc.h>
#if defined(MALLOC_USE_HASH)
MALLOC_USE_HASH(1);
#endif
#endif
#ifdef ERTS_THR_HAVE_SIG_FUNCS
static sigset_t thr_create_sigmask;
#endif
typedef struct {
#ifdef ERTS_THR_HAVE_SIG_FUNCS
    sigset_t saved_sigmask;
#endif
    int sched_bind_data;
} erts_thr_create_data_t;
static void *
thr_create_prepare(void)
{
    erts_thr_create_data_t *tcdp;
    tcdp = erts_alloc(ERTS_ALC_T_TMP, sizeof(erts_thr_create_data_t));
#ifdef ERTS_THR_HAVE_SIG_FUNCS
    erts_thr_sigmask(SIG_BLOCK, &thr_create_sigmask, &tcdp->saved_sigmask);
#endif
    tcdp->sched_bind_data = erts_sched_bind_atthrcreate_prepare();
    return (void *) tcdp;
}
static void
thr_create_cleanup(void *vtcdp)
{
    erts_thr_create_data_t *tcdp = (erts_thr_create_data_t *) vtcdp;
    erts_sched_bind_atthrcreate_parent(tcdp->sched_bind_data);
#ifdef ERTS_THR_HAVE_SIG_FUNCS
    erts_thr_sigmask(SIG_SETMASK, &tcdp->saved_sigmask, NULL);
#endif
    erts_free(ERTS_ALC_T_TMP, tcdp);
}
#ifdef HAVE_LINUX_THP
static int
is_linux_thp_enabled(void)
{
    FILE *fp;
    char always[9], madvise[10], never[8];
    int matched;
    fp = fopen("/sys/kernel/mm/transparent_hugepage/enabled", "r");
    if (fp == NULL)
        return 0;
    matched = fscanf(fp, "%8s %9s %7s", always, madvise, never);
    fclose(fp);
    if (matched != 3 || strcmp("[never]", never) == 0)
        return 0;
    return strcmp("[always]", always) != 0 || strcmp("[madvise]", madvise) != 0;
}
#define ALIGN_DOWN(x, a) ((void*)(((UWord)(x)) & ~((a) - 1)))
static void
enable_linux_thp_for_text(void)
{
    FILE *fp;
    char *from, *to;
    extern char etext;
    if (sys_large_page_size == 0)
        return;
    if (is_linux_thp_enabled() == 0)
        return;
    fp = fopen("/proc/self/maps", "r");
    if (fp == NULL)
        return;
    while (fscanf(fp, "%p-%p %*[^\n]\n", &from, &to) == 2) {
        if (to < &etext)
            continue;
        if (from > &etext)
            break;
        if ((UWord)from % sys_large_page_size != 0)
            break;
        to = ALIGN_DOWN(to, sys_large_page_size);
        if (to - from < sys_large_page_size)
            break;
        madvise(from, to - from, MADV_HUGEPAGE);
    }
    fclose(fp);
}
#endif
static void
thr_create_prepare_child(void *vtcdp)
{
    erts_thr_create_data_t *tcdp = (erts_thr_create_data_t *) vtcdp;
#ifdef ERTS_ENABLE_LOCK_COUNT
    erts_lcnt_thread_setup();
#endif
    erts_sched_bind_atthrcreate_child(tcdp->sched_bind_data);
}
void
erts_sys_pre_init(void)
{
    erts_thr_init_data_t eid = ERTS_THR_INIT_DATA_DEF_INITER;
    erts_printf_add_cr_to_stdout = 1;
    erts_printf_add_cr_to_stderr = 1;
    eid.thread_create_child_func = thr_create_prepare_child;
    eid.thread_create_prepare_func = thr_create_prepare;
    eid.thread_create_parent_func = thr_create_cleanup;
    sys_page_size = get_page_size();
    sys_large_page_size = get_large_page_size();
#ifdef HAVE_LINUX_THP
    enable_linux_thp_for_text();
#endif
#ifdef ERTS_ENABLE_LOCK_COUNT
    erts_lcnt_pre_thr_init();
#endif
    erts_thr_init(&eid);
#ifdef ERTS_ENABLE_LOCK_COUNT
    erts_lcnt_post_thr_init();
#endif
#ifdef ERTS_ENABLE_LOCK_CHECK
    erts_lc_init();
#endif
#ifdef ERTS_DYN_LOCK_CHECK
    erts_dlc_init();
#endif
    erts_init_sys_time_sup();
    erts_atomic32_init_nob(&erts_break_requested, 0);
    erts_atomic32_init_nob(&have_prepared_crash_dump, 0);
    erts_atomic_init_nob(&sys_misc_mem_sz, 0);
    {
      int fd;
      if ((fd = open("/dev/null", O_RDONLY)) != 0)
	close(fd);
      while (fd < 3) {
	fd = open("/dev/null", O_WRONLY);
      }
      close(fd);
    }
    crashdump_companion_cube_fd = open("/dev/null", O_RDONLY);
}
void erts_sys_scheduler_init(void) {
    sys_thread_init_signal_stack();
}
void
erl_sys_init(void)
{
#ifdef USE_SETLINEBUF
    setlinebuf(stdout);
#else
    setvbuf(stdout, (char *)NULL, _IOLBF, BUFSIZ);
#endif
    erts_sys_init_float();
    if (isatty(0)) {
	tcgetattr(0,&erl_sys_initial_tty_mode);
    }
}
SIGFUNC sys_signal(int sig, SIGFUNC func)
{
    struct sigaction act, oact;
    int extra_flags = 0;
    sigemptyset(&act.sa_mask);
#if (defined(BEAMASM) && defined(NATIVE_ERLANG_STACK))
    if (func != SIG_DFL && func != SIG_IGN) {
        extra_flags |= SA_ONSTACK;
    }
#endif
    act.sa_flags = extra_flags;
    act.sa_handler = func;
    sigaction(sig, &act, &oact);
    return oact.sa_handler;
}
#undef  sigprocmask
#define sigprocmask erts_thr_sigmask
void sys_sigblock(int sig)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, sig);
    sigprocmask(SIG_BLOCK, &mask, (sigset_t *)NULL);
}
void sys_sigrelease(int sig)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, sig);
    sigprocmask(SIG_UNBLOCK, &mask, (sigset_t *)NULL);
}
#ifdef ERTS_HAVE_TRY_CATCH
void erts_sys_sigsegv_handler(int signo) {
    if (signo == SIGSEGV) {
        longjmp(erts_sys_sigsegv_jmp, 1);
    }
}
#endif
int
erts_sys_is_area_readable(char *start, char *stop) {
#ifdef ADDRESS_SANITIZER
    return __asan_region_is_poisoned(start, stop-start) == NULL;
#else
    int fds[2];
    if (!pipe(fds)) {
        int res = write(fds[1], start, (char*)stop - (char*)start);
        if (res == -1) {
            close(fds[0]);
            close(fds[1]);
            return 0;
        }
        close(fds[0]);
        close(fds[1]);
        return 1;
    }
    return 0;
#endif
}
static ERTS_INLINE int
prepare_crash_dump(int secs)
{
#define NUFBUF (3)
    int i;
    char env[21];
    size_t envsz;
    DeclareTmpHeapNoproc(heap,NUFBUF);
    Port *heart_port;
    Eterm *hp = heap;
    Eterm list = NIL;
    int has_heart = 0;
    UseTmpHeapNoproc(NUFBUF);
    if (ERTS_PREPARED_CRASH_DUMP)
	return 0;
    heart_port = erts_get_heart_port();
    if (secs >= 0) {
	alarm((unsigned int)secs);
    }
    erts_emergency_close_ports();
    if (heart_port) {
	has_heart = 1;
	list = CONS(hp, make_small(8), list); hp += 2;
	erts_port_output(NULL, ERTS_PORT_SIG_FLG_FORCE_IMM_CALL, heart_port,
			 heart_port->common.id, list, NULL);
    }
    close(crashdump_companion_cube_fd);
    envsz = sizeof(env);
    i = erts_sys_explicit_8bit_getenv("ERL_CRASH_DUMP_NICE", env, &envsz);
    if (i != 0) {
	int nice_val;
	nice_val = (i != 1) ? 0 : atoi(env);
	if (nice_val > 39) {
	    nice_val = 39;
	}
	erts_silence_warn_unused_result(nice(nice_val));
    }
    UnUseTmpHeapNoproc(NUFBUF);
#undef NUFBUF
    return has_heart;
}
int erts_sys_prepare_crash_dump(int secs)
{
    return prepare_crash_dump(secs);
}
static void signal_notify_requested(Eterm type) {
    Process* p = NULL;
    Eterm msg, *hp;
    ErtsProcLocks locks = 0;
    ErlOffHeap *ohp;
    Eterm id = erts_whereis_name_to_id(NULL, am_erl_signal_server);
    if ((p = (erts_pid2proc_opt(NULL, 0, id, 0, ERTS_P2P_FLG_INC_REFC))) != NULL) {
        ErtsMessage *msgp = erts_alloc_message_heap(p, &locks, 3, &hp, &ohp);
        msg = TUPLE2(hp, am_notify, type);
        erts_queue_message(p, locks, msgp, msg, am_system);
        if (locks)
            erts_proc_unlock(p, locks);
        erts_proc_dec_refc(p);
    }
}
static ERTS_INLINE void
break_requested(void)
{
  if (ERTS_BREAK_REQUESTED)
      erts_exit(ERTS_INTR_EXIT, "");
  ERTS_SET_BREAK_REQUESTED;
  erts_aux_thread_poke();
}
static RETSIGTYPE request_break(int signum)
{
    smp_sig_notify(signum);
}
#ifdef ETHR_UNUSABLE_SIGUSRX
#warning "Unusable SIGUSR1 & SIGUSR2. Disabling use of these signals"
#else
#ifdef ERTS_SYS_SUSPEND_SIGNAL
void
sys_thr_suspend(erts_tid_t tid) {
    erts_thr_kill(tid, ERTS_SYS_SUSPEND_SIGNAL);
}
void
sys_thr_resume(erts_tid_t tid) {
    int i = 0, res;
    do {
        res = write(sig_suspend_fds[1],&i,sizeof(i));
    } while (res < 0 && errno == EAGAIN);
}
#endif
#ifdef ERTS_SYS_SUSPEND_SIGNAL
#if (defined(SIG_SIGSET) || defined(SIG_SIGNAL))
static RETSIGTYPE suspend_signal(void)
#else
static RETSIGTYPE suspend_signal(int signum)
#endif
{
    int res, buf[1], tmp_errno = errno;
    do {
        res = read(sig_suspend_fds[0], buf, sizeof(int));
    } while (res < 0 && errno == EINTR);
    errno = tmp_errno;
}
#endif
#endif
static ERTS_INLINE int
signalterm_to_signum(Eterm signal)
{
    switch (signal) {
    case am_sighup:  return SIGHUP;
    case am_sigquit: return SIGQUIT;
    case am_sigabrt: return SIGABRT;
    case am_sigalrm: return SIGALRM;
    case am_sigterm: return SIGTERM;
    case am_sigusr1: return SIGUSR1;
    case am_sigusr2: return SIGUSR2;
    case am_sigchld: return SIGCHLD;
    case am_sigstop: return SIGSTOP;
    case am_sigtstp: return SIGTSTP;
    case am_sigcont: return SIGCONT;
    case am_sigwinch: return SIGWINCH;
#ifdef SIGINFO
    case am_siginfo: return SIGINFO;
#endif
    default:         return 0;
    }
}
static ERTS_INLINE Eterm
signum_to_signalterm(int signum)
{
    switch (signum) {
    case SIGHUP:  return am_sighup;
    case SIGQUIT: return am_sigquit;
    case SIGABRT: return am_sigabrt;
    case SIGALRM: return am_sigalrm;
    case SIGTERM: return am_sigterm;
    case SIGUSR1: return am_sigusr1;
    case SIGUSR2: return am_sigusr2;
    case SIGCHLD: return am_sigchld;
    case SIGSTOP: return am_sigstop;
    case SIGTSTP: return am_sigtstp;
    case SIGCONT: return am_sigcont;
    case SIGWINCH: return am_sigwinch;
#ifdef SIGINFO
    case SIGINFO: return am_siginfo;
#endif
    default:      return am_error;
    }
}
static RETSIGTYPE generic_signal_handler(int signum)
{
    smp_sig_notify(signum);
}
int erts_set_signal(Eterm signal, Eterm type) {
    int signum;
    if ((signum = signalterm_to_signum(signal)) > 0) {
        if (type == am_ignore) {
            sys_signal(signum, SIG_IGN);
        } else if (type == am_default) {
            sys_signal(signum, SIG_DFL);
        } else {
            sys_signal(signum, generic_signal_handler);
        }
        return 1;
    }
    return 0;
}
void erts_set_ignore_break(void) {
    sys_signal(SIGINT,  SIG_IGN);
    sys_signal(SIGQUIT, SIG_IGN);
    sys_signal(SIGTSTP, SIG_IGN);
}
void erts_replace_intr(void) {
  struct termios mode;
  if (isatty(0)) {
    tcgetattr(0, &mode);
    mode.c_cc[VINTR] = 0;
    tcsetattr(0, TCSANOW, &mode);
    replace_intr = 1;
  }
}
void init_break_handler(void)
{
   sys_signal(SIGINT,  request_break);
   sys_signal(SIGQUIT, generic_signal_handler);
}
void sys_init_suspend_handler(void)
{
#ifdef ERTS_SYS_SUSPEND_SIGNAL
   sys_signal(ERTS_SYS_SUSPEND_SIGNAL, suspend_signal);
#endif
}
void
erts_sys_unix_later_init(void)
{
    sys_signal(SIGTERM, generic_signal_handler);
#ifndef ETHR_UNUSABLE_SIGUSRX
   sys_signal(SIGUSR1, generic_signal_handler);
#endif
    sys_signal(SIGCHLD, SIG_IGN);
}
int sys_max_files(void)
{
   return max_files;
}
char os_type[] = "unix";
static int
get_number(char **str_ptr)
{
    char* s = *str_ptr;
    char* dot;
    if (!isdigit((int) *s))
	return 0;
    if ((dot = strchr(s, '.')) == NULL) {
	*str_ptr = s+strlen(s);
	return atoi(s);
    } else {
	*dot = '\0';
	*str_ptr = dot+1;
	return atoi(s);
    }
}
void os_flavor(char* namebuf, unsigned size) {
    struct utsname uts;
    char* s;
    (void) uname(&uts);
    for (s = uts.sysname; *s; s++) {
	if (isupper((int) *s)) {
	    *s = tolower((int) *s);
	}
    }
    strcpy(namebuf, uts.sysname);
}
void os_version(int *pMajor, int *pMinor, int *pBuild) {
    struct utsname uts;
    char* release;
    (void) uname(&uts);
#ifdef _AIX
    *pMajor = atoi(uts.version);
    *pMinor = atoi(uts.release);
    *pBuild = 0;
#else
    release = uts.release;
    *pMajor = get_number(&release);
    *pMinor = get_number(&release);
    *pBuild = get_number(&release);
#endif
}
void erts_do_break_handling(void)
{
    struct termios temp_mode;
    int saved = 0;
    erts_thr_progress_block();
    if (using_oldshell && !replace_intr) {
      SET_BLOCKING(1);
    }
    else if (isatty(0)) {
      tcgetattr(0,&temp_mode);
      tcsetattr(0,TCSANOW,&erl_sys_initial_tty_mode);
      saved = 1;
    }
    do_break();
    ERTS_UNSET_BREAK_REQUESTED;
    fflush(stdout);
    if (using_oldshell && !replace_intr) {
      SET_NONBLOCKING(1);
    }
    else if (saved) {
      tcsetattr(0,TCSANOW,&temp_mode);
    }
    erts_thr_progress_unblock();
}
void sys_get_pid(char *buffer, size_t buffer_size){
    pid_t p = getpid();
    erts_snprintf(buffer, buffer_size, "%lu",(unsigned long) p);
}
int sys_get_hostname(char *buf, size_t size)
{
    return gethostname(buf, size);
}
void sys_init_io(void) { }
void erts_sys_alloc_init(void) { }
extern const char pre_loaded_code[];
extern Preload pre_loaded[];
#if ERTS_HAVE_ERTS_SYS_ALIGNED_ALLOC
void *erts_sys_aligned_alloc(UWord alignment, UWord size)
{
#ifdef HAVE_POSIX_MEMALIGN
    void *ptr = NULL;
    int error;
    ASSERT(alignment && (alignment & (alignment-1)) == 0);
    error = posix_memalign(&ptr, (size_t) alignment, (size_t) size);
#if HAVE_ERTS_MSEG
    if (error || !ptr) {
	erts_mseg_clear_cache();
	error = posix_memalign(&ptr, (size_t) alignment, (size_t) size);
    }
#endif
    if (error) {
	errno = error;
	return NULL;
    }
    if (!ptr)
	errno = ENOMEM;
    ASSERT(!ptr || (((UWord) ptr) & (alignment - 1)) == 0);
    return ptr;
#else
#  error "Missing erts_sys_aligned_alloc() implementation"
#endif
}
void erts_sys_aligned_free(UWord alignment, void *ptr)
{
    ASSERT(alignment && (alignment & (alignment-1)) == 0);
    free(ptr);
}
void *erts_sys_aligned_realloc(UWord alignment, void *ptr, UWord size, UWord old_size)
{
    void *new_ptr = erts_sys_aligned_alloc(alignment, size);
    if (new_ptr) {
	UWord copy_size = old_size < size ? old_size : size;
	sys_memcpy(new_ptr, ptr, (size_t) copy_size);
	erts_sys_aligned_free(alignment, ptr);
    }
    return new_ptr;
}
#endif
void *erts_sys_alloc(ErtsAlcType_t t, void *x, Uint sz)
{
    void *res = malloc((size_t) sz);
#if HAVE_ERTS_MSEG
    if (!res) {
	erts_mseg_clear_cache();
	return malloc((size_t) sz);
    }
#endif
    return res;
}
void *erts_sys_realloc(ErtsAlcType_t t, void *x, void *p, Uint sz)
{
    void *res = realloc(p, (size_t) sz);
#if HAVE_ERTS_MSEG
    if (!res) {
	erts_mseg_clear_cache();
	return realloc(p, (size_t) sz);
    }
#endif
    return res;
}
void erts_sys_free(ErtsAlcType_t t, void *x, void *p)
{
    free(p);
}
Preload*
sys_preloaded(void)
{
    return pre_loaded;
}
unsigned char*
sys_preload_begin(Preload* p)
{
    return p->code;
}
void sys_preload_end(Preload* p)
{
}
int sys_get_key(int fd) {
    int c, ret;
    unsigned char rbuf[64] = {0};
    fd_set fds;
    fflush(stdout);
    FD_ZERO(&fds);
    FD_SET(fd,&fds);
    ret = select(fd+1, &fds, NULL, NULL, NULL);
    if (ret == 1) {
        do {
            c = read(fd,rbuf,64);
        } while (c < 0 && errno == EAGAIN);
        if (c <= 0)
            return c;
    }
    return rbuf[0];
}
extern int erts_initialized;
void
erl_assert_error(const char* expr, const char* func, const char* file, int line)
{
    fflush(stdout);
    fprintf(stderr, "%s:%d:%s() Assertion failed: %s\n",
            file, line, func, expr);
    fflush(stderr);
    abort();
}
#ifdef DEBUG
void
erl_debug(char* fmt, ...)
{
    char sbuf[1024];
    va_list va;
    if (debug_log) {
	va_start(va, fmt);
	vsprintf(sbuf, fmt, va);
	va_end(va);
	fprintf(stderr, "%s", sbuf);
    }
}
#endif
static erts_tid_t sig_dispatcher_tid;
static void
smp_sig_notify(int signum)
{
    int res;
    do {
	res = write(sig_notify_fds[1], &signum, sizeof(int));
    } while (res < 0 && errno == EINTR);
    if (res != sizeof(int)) {
	char msg[] =
	    "smp_sig_notify(): Failed to notify signal-dispatcher thread "
	    "about received signal";
	erts_silence_warn_unused_result(write(2, msg, sizeof(msg)));
	abort();
    }
}
static void *
signal_dispatcher_thread_func(void *unused)
{
#ifdef ERTS_ENABLE_LOCK_CHECK
    erts_lc_set_thread_name("signal_dispatcher");
#endif
    while (1) {
        union {int signum; char buf[4];} sb;
        Eterm signal;
	int res, i = 0;
        do {
            res = read(sig_notify_fds[0], (void *) &sb.buf[i], sizeof(int) - i);
            i += res > 0 ? res : 0;
        } while ((i < sizeof(int) && res >= 0) || (res < 0 && errno == EINTR));
	if (res < 0) {
	    erts_exit(ERTS_ABORT_EXIT,
		     "signal-dispatcher thread got unexpected error: %s (%d)\n",
		     erl_errno_id(errno),
		     errno);
	}
        switch (sb.signum) {
            case 0: continue;
            case SIGINT:
                break_requested();
                break;
            default:
                if ((signal = signum_to_signalterm(sb.signum)) == am_error) {
                    erts_exit(ERTS_ABORT_EXIT,
                            "signal-dispatcher thread received unknown "
                            "signal notification: '%d'\n",
                            sb.signum);
                }
                signal_notify_requested(signal);
        }
        ERTS_LC_ASSERT(!erts_thr_progress_is_blocking());
    }
    return NULL;
}
static void
init_smp_sig_notify(void)
{
    erts_thr_opts_t thr_opts = ERTS_THR_OPTS_DEFAULT_INITER;
    thr_opts.detached = 1;
    thr_opts.name = "erts_ssig_disp";
    if (pipe(sig_notify_fds) < 0) {
	erts_exit(ERTS_ABORT_EXIT,
		 "Failed to create signal-dispatcher pipe: %s (%d)\n",
		 erl_errno_id(errno),
		 errno);
    }
    erts_thr_create(&sig_dispatcher_tid,
			signal_dispatcher_thread_func,
			NULL,
			&thr_opts);
}
static void
init_smp_sig_suspend(void) {
#ifdef ERTS_SYS_SUSPEND_SIGNAL
  if (pipe(sig_suspend_fds) < 0) {
    erts_exit(ERTS_ABORT_EXIT,
	     "Failed to create sig_suspend pipe: %s (%d)\n",
	     erl_errno_id(errno),
	     errno);
  }
#endif
}
#ifdef __DARWIN__
int erts_darwin_main_thread_pipe[2];
int erts_darwin_main_thread_result_pipe[2];
static void initialize_darwin_main_thread_pipes(void)
{
    if (pipe(erts_darwin_main_thread_pipe) < 0 ||
	pipe(erts_darwin_main_thread_result_pipe) < 0) {
	erts_exit(ERTS_ERROR_EXIT,"Fatal error initializing Darwin main thread stealing");
    }
}
#endif
void
erts_sys_main_thread(void)
{
#ifdef __DARWIN__
    initialize_darwin_main_thread_pipes();
#else
#ifdef ERTS_ENABLE_LOCK_CHECK
    erts_lc_set_thread_name("main");
#endif
#endif
    smp_sig_notify(0);
#ifdef __DARWIN__
    while (1) {
	fd_set readfds;
	int res;
	FD_ZERO(&readfds);
	FD_SET(erts_darwin_main_thread_pipe[0], &readfds);
	res = select(erts_darwin_main_thread_pipe[0] + 1, &readfds, NULL, NULL, NULL);
	if (res > 0 && FD_ISSET(erts_darwin_main_thread_pipe[0],&readfds)) {
	    void* (*func)(void*);
	    void* arg;
	    void *resp;
            res = read(erts_darwin_main_thread_pipe[0],&func,sizeof(void* (*)(void*)));
            if (res != sizeof(void* (*)(void*)))
                break;
            res = read(erts_darwin_main_thread_pipe[0],&arg,sizeof(void*));
            if (res != sizeof(void*))
                break;
	    resp = (*func)(arg);
	    write(erts_darwin_main_thread_result_pipe[1],&resp,sizeof(void *));
	}
        if (res == -1 && errno != EINTR)
            break;
    }
#endif
    while (1) {
#ifdef DEBUG
	int res =
#else
	(void)
#endif
	    select(0, NULL, NULL, NULL, NULL);
	ASSERT(res < 0);
	ASSERT(errno == EINTR);
    }
}
void
erl_sys_args(int* argc, char** argv)
{
    ASSERT(argc && argv);
    max_files = erts_check_io_max_files();
    init_smp_sig_notify();
    init_smp_sig_suspend();
    erts_sys_env_init();
}