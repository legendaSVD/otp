#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#define WANT_NONBLOCKING
#include "erl_driver.h"
#include "sys_uds.h"
#include "erl_term.h"
#include "erl_child_setup.h"
#undef ERTS_GLB_INLINE_INCL_FUNC_DEF
#define ERTS_GLB_INLINE_INCL_FUNC_DEF 1
#include "hash.h"
#define SET_CLOEXEC(fd) fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC)
#if defined(__ANDROID__)
#define SHELL "/system/bin/sh"
#else
#define SHELL "/bin/sh"
#endif
#if !defined(MSG_DONTWAIT) && defined(MSG_NONBLOCK)
#define MSG_DONTWAIT MSG_NONBLOCK
#endif
#ifdef HARD_DEBUG
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, "%d:" fmt "\r\n", getpid(), ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif
#ifdef __clang_analyzer__
#  undef FD_ZERO
#  define FD_ZERO(FD_SET_PTR) memset(FD_SET_PTR, 0, sizeof(fd_set))
#endif
static char abort_reason[200];
static void ABORT(const char* fmt, ...)
{
    va_list arglist;
    va_start(arglist, fmt);
    vsprintf(abort_reason, fmt, arglist);
    fprintf(stderr, "erl_child_setup: %s\r\n", abort_reason);
    va_end(arglist);
    abort();
}
#ifdef DEBUG
void
erl_assert_error(const char* expr, const char* func, const char* file, int line)
{
    fflush(stdout);
    fprintf(stderr, "%s:%d:%s() Assertion failed: %s\n",
            file, line, func, expr);
    fflush(stderr);
    abort();
}
#endif
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
static ssize_t read_all(int fd, char *buff, size_t size) {
    ssize_t res, pos = 0;
    do {
        if ((res = read(fd, buff + pos, size - pos)) < 0) {
            if (errno == ERRNO_BLOCK || errno == EINTR)
                continue;
            return res;
        }
        if (res == 0) {
            errno = EPIPE;
            return -1;
        }
        pos += res;
    } while(size - pos != 0);
    return pos;
}
static ssize_t write_all(int fd, const char *buff, size_t size) {
    ssize_t res, pos = 0;
    do {
        if ((res = write(fd, buff + pos, size - pos)) < 0) {
            if (errno == ERRNO_BLOCK || errno == EINTR)
                continue;
            return res;
        }
        if (res == 0) {
            errno = EPIPE;
            return -1;
        }
        pos += res;
    } while (size - pos != 0);
    return pos;
}
static void add_os_pid_to_port_id_mapping(Eterm, pid_t);
static Eterm get_port_id(pid_t);
static int forker_hash_init(void);
static int max_files = -1;
static int sigchld_pipe[2];
static int
start_new_child(int pipes[])
{
    struct sigaction sa;
    int errln = -1;
    int size, i;
    char *buff, *o_buff;
    char *cmd, *cwd, *wd, **new_environ, **args = NULL;
    Sint32 cnt, flags;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGTERM, &sa, 0) == -1) {
        perror(NULL);
        exit(1);
    }
    if (read_all(pipes[0], (char*)&size, sizeof(size)) <= 0) {
        errln = __LINE__;
        goto child_error;
    }
    buff = malloc(size);
    DEBUG_PRINT("size = %d", size);
    if (read_all(pipes[0], buff, size) <= 0) {
        errln = __LINE__;
        goto child_error;
    }
    o_buff = buff;
    flags = get_int32(buff);
    buff += sizeof(flags);
    DEBUG_PRINT("flags = %d", flags);
    cmd = buff;
    buff += strlen(buff) + 1;
    cwd = buff;
    buff += strlen(buff) + 1;
    if (*buff == '\0') {
        wd = NULL;
    } else {
        wd = buff;
        buff += strlen(buff) + 1;
    }
    buff++;
    DEBUG_PRINT("wd = %s", wd);
    cnt = get_int32(buff);
    buff += sizeof(cnt);
    new_environ = malloc(sizeof(char*)*(cnt + 1));
    DEBUG_PRINT("env_len = %d", cnt);
    for (i = 0; i < cnt; i++, buff++) {
        new_environ[i] = buff;
        while(*buff != '\0') buff++;
    }
    new_environ[cnt] = NULL;
    if (o_buff + size != buff) {
        cnt = get_int32(buff);
        buff += sizeof(cnt);
        args = malloc(sizeof(char*)*(cnt + 1));
        for (i = 0; i < cnt; i++, buff++) {
            args[i] = buff;
            while(*buff != '\0') buff++;
        }
        args[cnt] = NULL;
    }
    if (o_buff + size != buff) {
        errno = EINVAL;
        errln = __LINE__;
        fprintf(stderr,"erl_child_setup: failed with protocol "
                "error %d on line %d", errno, errln);
        abort();
    }
    DEBUG_PRINT("read ack");
    {
        ErtsSysForkerProto proto;
        if (read_all(pipes[0], (char*)&proto, sizeof(proto)) <= 0) {
            errln = __LINE__;
            goto child_error;
        }
        ASSERT(proto.action == ErtsSysForkerProtoAction_Ack);
    }
    DEBUG_PRINT("Set cwd to: '%s'",cwd);
    if (chdir(cwd) < 0) {
    }
    DEBUG_PRINT("Set wd to: '%s'",wd);
    if (wd && chdir(wd) < 0) {
        int err = errno;
        fprintf(stderr,"spawn: Could not cd to %s\r\n", wd);
        _exit(err);
    }
    DEBUG_PRINT("Do that forking business: '%s'",cmd);
    if (flags & FORKER_FLAG_USE_STDIO) {
        if (flags & FORKER_FLAG_DO_WRITE &&
            dup2(pipes[0], 0) < 0) {
            errln = __LINE__;
            goto child_error;
        }
        if (flags & FORKER_FLAG_DO_READ &&
            dup2(pipes[1], 1) < 0) {
            errln = __LINE__;
            goto child_error;
        }
    }
    else {
        if (flags & FORKER_FLAG_DO_READ && dup2(pipes[1], 4) < 0) {
            errln = __LINE__;
            goto child_error;
        }
        if (flags & FORKER_FLAG_DO_WRITE && dup2(pipes[0], 3) < 0) {
            errln = __LINE__;
            goto child_error;
        }
    }
    if (dup2(pipes[2], 2) < 0) {
        errln = __LINE__;
        goto child_error;
    }
#if defined(USE_SETPGRP_NOARGS)
    (void) setpgrp();
#elif defined(USE_SETPGRP)
    (void) setpgrp(0, getpid());
#else
    (void) setsid();
#endif
    close(pipes[0]);
    close(pipes[1]);
    close(pipes[2]);
    sys_sigrelease(SIGCHLD);
    if (args) {
        execve(cmd, args, new_environ);
    } else {
        execle(SHELL, "sh", "-c", cmd, (char *) NULL, new_environ);
    }
    DEBUG_PRINT("exec error: %d",errno);
    _exit(errno);
child_error:
    fprintf(stderr,"erl_child_setup: failed with error %d on line %d\r\n",
            errno, errln);
    _exit(errno);
}
static void handle_sigchld(int sig) {
    int buff[2], __preverrno = errno;
    ssize_t res;
    sys_sigblock(SIGCHLD);
    while ((buff[0] = waitpid((pid_t)(-1), buff+1, WNOHANG)) > 0) {
        if ((res = write_all(sigchld_pipe[1], (char*)buff, sizeof(buff))) <= 0)
            ABORT("Failed to write to sigchld_pipe (%d): %d (%d)", sigchld_pipe[1], res, errno);
        DEBUG_PRINT("Reap child %d (%d)", buff[0], buff[1]);
    }
    sys_sigrelease(SIGCHLD);
    errno = __preverrno;
}
#if defined(__ANDROID__)
static int system_properties_fd(void)
{
    static int fd = -2;
    char *env;
    if (fd != -2) return fd;
    env = getenv("ANDROID_PROPERTY_WORKSPACE");
    if (!env) {
        fd = -1;
        return -1;
    }
    fd = atoi(env);
    return fd;
}
#endif
static struct termios initial_tty_mode;
int
main(int argc, char *argv[])
{
    int uds_fd = 3, max_fd = 3;
#ifndef HAVE_CLOSEFROM
    int i;
    DIR *dir;
#endif
    struct sigaction sa;
    if (argc < 2 || sscanf(argv[1],"%d",&max_files) != 1) {
        ABORT("Invalid arguments to child_setup");
    }
#if defined(HAVE_CLOSEFROM)
    closefrom(4);
#else
    dir = opendir("/dev/fd");
    if (dir == NULL) {
        for (i = 4; i < max_files; i++)
#if defined(__ANDROID__)
            if (i != system_properties_fd())
#endif
            (void) close(i);
    } else {
        struct dirent *entry;
        int dir_fd = dirfd(dir);
        while ((entry = readdir(dir)) != NULL) {
            i = atoi(entry->d_name);
#if defined(__ANDROID__)
            if (i != system_properties_fd())
#endif
            if (i >= 4 && i != dir_fd)
                (void) close(i);
        }
        closedir(dir);
    }
#endif
    if (pipe(sigchld_pipe) < 0) {
        ABORT("Failed to setup sigchld pipe (%d)", errno);
    }
    SET_CLOEXEC(sigchld_pipe[0]);
    SET_CLOEXEC(sigchld_pipe[1]);
    max_fd = max_fd < sigchld_pipe[0] ? sigchld_pipe[0] : max_fd;
    sa.sa_handler = &handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror(NULL);
        exit(1);
    }
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGTERM, &sa, 0) == -1) {
        perror(NULL);
        exit(1);
    }
    forker_hash_init();
    SET_CLOEXEC(uds_fd);
    if (isatty(0) && isatty(1)) {
        ssize_t res = read_all(uds_fd, (char*)&initial_tty_mode, sizeof(struct termios));
        if (res <= 0) {
            ABORT("Failed to read initial_tty_mode: %d (%d)", res, errno);
        }
    }
    DEBUG_PRINT("Starting forker %d", max_files);
    while (1) {
        fd_set read_fds;
        int res;
        FD_ZERO(&read_fds);
        FD_SET(uds_fd, &read_fds);
        FD_SET(sigchld_pipe[0], &read_fds);
        DEBUG_PRINT("child_setup selecting on %d, %d (%d)",
                uds_fd, sigchld_pipe[0], max_fd);
        res = select(max_fd+1, &read_fds, NULL, NULL, NULL);
        if (res < 0) {
            if (errno == EINTR) continue;
            ABORT("Select failed: %d (%d)",res, errno);
        }
        if (FD_ISSET(uds_fd, &read_fds)) {
            int pipes[3], res, os_pid;
            ErtsSysForkerProto proto;
            errno = 0;
            if ((res = sys_uds_read(uds_fd, (char*)&proto, sizeof(proto),
                                    pipes, 3, MSG_DONTWAIT)) < 0) {
                if (errno == EINTR)
                    continue;
                if (isatty(0) && isatty(1)) {
                    tcsetattr(0,TCSANOW,&initial_tty_mode);
                }
                DEBUG_PRINT("erl_child_setup failed to read from uds: %d, %d", res, errno);
                _exit(0);
            }
            if (res == 0) {
                DEBUG_PRINT("uds was closed!");
                if (isatty(0) && isatty(1)) {
                    tcsetattr(0,TCSANOW,&initial_tty_mode);
                }
                _exit(0);
            }
            ASSERT(res == sizeof(proto));
            ASSERT(proto.action == ErtsSysForkerProtoAction_Start);
            sys_sigblock(SIGCHLD);
            errno = 0;
            os_pid = fork();
            if (os_pid == 0)
                start_new_child(pipes);
            add_os_pid_to_port_id_mapping(proto.u.start.port_id, os_pid);
            proto.action = ErtsSysForkerProtoAction_Go;
            proto.u.go.os_pid = os_pid;
            proto.u.go.error_number = errno;
            write_all(pipes[1], (char *)&proto, sizeof(proto));
#ifdef FORKER_PROTO_START_ACK
            proto.action = ErtsSysForkerProtoAction_StartAck;
            write_all(uds_fd, (char *)&proto, sizeof(proto));
#endif
            sys_sigrelease(SIGCHLD);
            close(pipes[0]);
            close(pipes[1]);
            close(pipes[2]);
        }
        if (FD_ISSET(sigchld_pipe[0], &read_fds)) {
            int ibuff[2];
            ErtsSysForkerProto proto;
            res = read_all(sigchld_pipe[0], (char *)ibuff, sizeof(ibuff));
            if (res <= 0) {
                ABORT("Failed to read from sigchld pipe: %d (%d)", res, errno);
            }
            proto.u.sigchld.port_id = get_port_id((pid_t)(ibuff[0]));
            if (proto.u.sigchld.port_id == THE_NON_VALUE)
                continue;
            proto.action = ErtsSysForkerProtoAction_SigChld;
            proto.u.sigchld.error_number = ibuff[1];
            DEBUG_PRINT("send sigchld to %d (errno = %d)", uds_fd, ibuff[1]);
            if (write_all(uds_fd, (char *)&proto, sizeof(proto)) < 0) {
                DEBUG_PRINT("Failed to write to uds: %d (%d)", uds_fd, errno);
            }
        }
    }
    return 1;
}
typedef struct exit_status {
    HashBucket hb;
    pid_t os_pid;
    Eterm port_id;
} ErtsSysExitStatus;
static Hash *forker_hash;
static void add_os_pid_to_port_id_mapping(Eterm port_id, pid_t os_pid)
{
    if (port_id != THE_NON_VALUE) {
        ErtsSysExitStatus es;
        es.os_pid = os_pid;
        es.port_id = port_id;
        hash_put(forker_hash, &es);
    }
}
static Eterm get_port_id(pid_t os_pid)
{
    ErtsSysExitStatus est, *es;
    Eterm port_id;
    est.os_pid = os_pid;
    es = hash_remove(forker_hash, &est);
    if (!es) return THE_NON_VALUE;
    port_id = es->port_id;
    free(es);
    return port_id;
}
static int fcmp(void *a, void *b)
{
    ErtsSysExitStatus *sa = a;
    ErtsSysExitStatus *sb = b;
    return !(sa->os_pid == sb->os_pid);
}
static HashValue fhash(void *e)
{
    ErtsSysExitStatus *se = e;
    Uint32 val = se->os_pid;
    val = (val+0x7ed55d16) + (val<<12);
    val = (val^0xc761c23c) ^ (val>>19);
    val = (val+0x165667b1) + (val<<5);
    val = (val+0xd3a2646c) ^ (val<<9);
    val = (val+0xfd7046c5) + (val<<3);
    val = (val^0xb55a4f09) ^ (val>>16);
    return val;
}
static void *falloc(void *e)
{
    ErtsSysExitStatus *se = e;
    ErtsSysExitStatus *ne = malloc(sizeof(ErtsSysExitStatus));
    ne->os_pid = se->os_pid;
    ne->port_id = se->port_id;
    return ne;
}
static void *meta_alloc(int type, size_t size) { return malloc(size); }
static void meta_free(int type, void *p)       { free(p); }
static int forker_hash_init(void)
{
    HashFunctions forker_hash_functions;
    forker_hash_functions.hash = fhash;
    forker_hash_functions.cmp = fcmp;
    forker_hash_functions.alloc = falloc;
    forker_hash_functions.free = free;
    forker_hash_functions.meta_alloc = meta_alloc;
    forker_hash_functions.meta_free  = meta_free;
    forker_hash_functions.meta_print = NULL;
    forker_hash = hash_new(0, "forker_hash",
                           16, forker_hash_functions);
    return 1;
}