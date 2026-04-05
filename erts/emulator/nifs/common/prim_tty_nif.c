#define STATIC_ERLANG_NIF 1
#ifndef WANT_NONBLOCKING
#define WANT_NONBLOCKING
#endif
#include "config.h"
#include "sys.h"
#include "erl_nif.h"
#include "erl_driver.h"
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <stdio.h>
#include <signal.h>
#include <locale.h>
#if defined(HAVE_TERMCAP)
#include <termios.h>
#if defined(HAVE_NCURSES_CURSES_H)
#include <ncurses/curses.h>
#include <ncurses/term.h>
#elif defined(HAVE_CURSES_H) && defined(HAVE_TERM_H)
#include <curses.h>
#include <term.h>
#else
#undef HAVE_TERMCAP
#endif
#endif
#ifndef __WIN32__
#include <unistd.h>
#include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif
#ifdef HAVE_POLL_H
#include <poll.h>
#endif
#if defined IOV_MAX
#define MAXIOV IOV_MAX
#elif defined UIO_MAXIOV
#define MAXIOV UIO_MAXIOV
#else
#define MAXIOV 16
#endif
#if !defined(HAVE_SETLOCALE) || !defined(HAVE_NL_LANGINFO) || !defined(HAVE_LANGINFO_H)
#define PRIMITIVE_UTF8_CHECK 1
#else
#include <langinfo.h>
#endif
#ifdef VALGRIND
#  include <valgrind/memcheck.h>
#endif
#define DEF_HEIGHT 24
#define DEF_WIDTH 80
enum TTYState {
    unavailable,
    disabled,
    enabled
};
typedef struct {
#ifdef __WIN32__
    HANDLE ofd;
    HANDLE ifd;
    DWORD dwOriginalOutMode;
    DWORD dwOriginalInMode;
    DWORD dwOutMode;
    DWORD dwInMode;
#else
    int ofd;
    int ifd;
#endif
    ErlNifPid self;
    ErlNifPid reader;
    enum TTYState tty;
#ifdef HAVE_TERMCAP
    struct termios tty_smode;
    struct termios tty_rmode;
#endif
} TTYResource;
#ifdef HARD_DEBUG
static FILE *logFile = NULL;
#define debug(fmt, ...) do { if (logFile) { erts_fprintf(logFile, fmt, __VA_ARGS__); fflush(logFile); } } while(0)
#else
#define debug(...) do { } while(0)
#endif
static ErlNifResourceType *tty_rt;
static ERL_NIF_TERM isatty_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM tty_create_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM tty_init_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM tty_is_open(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM setlocale_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM tty_select_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM tty_write_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM tty_encoding_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM tty_read_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM isprint_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM wcwidth_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM wcswidth_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM sizeof_wchar_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM tty_window_size_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM tty_setupterm_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM tty_tigetnum_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM tty_tigetflag_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM tty_tinfo_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM tty_tigetstr_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM tty_tputs_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ErlNifFunc nif_funcs[] = {
    {"isatty", 1, isatty_nif},
    {"tty_create", 1, tty_create_nif},
    {"tty_init", 2, tty_init_nif},
    {"setlocale", 1, setlocale_nif},
    {"tty_is_open", 2, tty_is_open},
    {"tty_select", 2, tty_select_nif},
    {"tty_window_size", 1, tty_window_size_nif},
    {"write_nif", 2, tty_write_nif, ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"tty_encoding", 1, tty_encoding_nif},
    {"read_nif", 3, tty_read_nif, ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"isprint", 1, isprint_nif},
    {"wcwidth", 1, wcwidth_nif},
    {"wcswidth", 1, wcswidth_nif},
    {"sizeof_wchar", 0, sizeof_wchar_nif},
    {"setupterm_nif", 0, tty_setupterm_nif},
    {"tigetnum_nif", 1, tty_tigetnum_nif},
    {"tigetflag_nif", 1, tty_tigetflag_nif},
    {"tinfo_nif", 0, tty_tinfo_nif},
    {"tigetstr_nif", 1, tty_tigetstr_nif},
    {"tputs_nif", 2, tty_tputs_nif}
};
static int load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info);
static int upgrade(ErlNifEnv* env, void** priv_data, void** old_priv_data, ERL_NIF_TERM load_info);
static void unload(ErlNifEnv* env, void* priv_data);
ERL_NIF_INIT(prim_tty, nif_funcs, load, NULL, upgrade, unload)
#define ATOMS                                   \
    ATOM_DECL(canon);                           \
    ATOM_DECL(echo);                            \
    ATOM_DECL(ebadf);                           \
    ATOM_DECL(undefined);                       \
    ATOM_DECL(error);                           \
    ATOM_DECL(true);                            \
    ATOM_DECL(stdout);                          \
    ATOM_DECL(ok);                              \
    ATOM_DECL(input);                           \
    ATOM_DECL(false);                           \
    ATOM_DECL(stdin);                           \
    ATOM_DECL(stdout);                          \
    ATOM_DECL(stderr);                          \
    ATOM_DECL(select);                          \
    ATOM_DECL(raw);                          \
    ATOM_DECL(sig);
#define ATOM_DECL(A) static ERL_NIF_TERM atom_##A
ATOMS
#undef ATOM_DECL
static ERL_NIF_TERM make_error(ErlNifEnv *env, ERL_NIF_TERM reason) {
    return enif_make_tuple2(env, atom_error, reason);
}
static ERL_NIF_TERM make_enotsup(ErlNifEnv *env) {
    return make_error(env, enif_make_atom(env, "enotsup"));
}
static ERL_NIF_TERM make_errno(ErlNifEnv *env) {
#ifdef __WIN32__
    return enif_make_atom(env, last_error());
#else
    return enif_make_atom(env, erl_errno_id(errno));
#endif
}
static ERL_NIF_TERM make_errno_error(ErlNifEnv *env, const char *function) {
    return make_error(
        env, enif_make_tuple2(
            env, enif_make_atom(env, function), make_errno(env)));
}
static int tty_get_fd(ErlNifEnv *env, ERL_NIF_TERM atom, int *fd) {
    if (enif_is_identical(atom, atom_stdout)) {
        *fd = fileno(stdout);
    } else if (enif_is_identical(atom, atom_stdin)) {
        *fd =  fileno(stdin);
    } else if (enif_is_identical(atom, atom_stderr)) {
        *fd =  fileno(stderr);
    } else {
        return 0;
    }
    return 1;
}
#ifdef __WIN32__
static HANDLE tty_get_handle(ErlNifEnv *env, ERL_NIF_TERM atom) {
    HANDLE handle = INVALID_HANDLE_VALUE;
    int fd;
    if (tty_get_fd(env, atom, &fd)) {
        switch (fd) {
            case 0: handle = GetStdHandle(STD_INPUT_HANDLE); break;
            case 1: handle = GetStdHandle(STD_OUTPUT_HANDLE); break;
            case 2: handle = GetStdHandle(STD_ERROR_HANDLE); break;
        }
    }
    return handle;
}
#endif
static ERL_NIF_TERM isatty_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
#ifdef __WIN32__
    HANDLE handle = tty_get_handle(env, argv[0]);
    if (handle == INVALID_HANDLE_VALUE)
        return atom_ebadf;
    switch (GetFileType(handle)) {
        case FILE_TYPE_CHAR: return atom_true;
        case FILE_TYPE_PIPE:
        case FILE_TYPE_DISK: return atom_false;
        default: return atom_ebadf;
    }
#else
    int fd;
    if (tty_get_fd(env, argv[0], &fd)) {
        if (isatty(fd)) {
            return atom_true;
        } else if (errno == EINVAL || errno == ENOTTY) {
            return atom_false;
        }
        else {
            return atom_ebadf;
        }
    }
#endif
    return enif_make_badarg(env);
}
static ERL_NIF_TERM tty_encoding_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
#ifdef __WIN32__
    TTYResource *tty;
    if (!enif_get_resource(env, argv[0], tty_rt, (void **)&tty))
        return enif_make_badarg(env);
    if (GetFileType(tty->ifd) == FILE_TYPE_CHAR)
        return enif_make_tuple2(env, enif_make_atom(env, "utf16"),
                                enif_make_atom(env, "little"));
#endif
    return enif_make_atom(env, "utf8");
}
static ERL_NIF_TERM isprint_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    int i;
    if (enif_get_int(env, argv[0], &i)) {
        ASSERT(i >= 0 && i < 256);
        return isprint((char)i) ? atom_true : atom_false;
    }
    return enif_make_badarg(env);
}
static ERL_NIF_TERM wcwidth_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    int i;
    if (enif_get_int(env, argv[0], &i)) {
#ifndef __WIN32__
        int width;
        ASSERT(i > 0 && i < (1l << 21));
        width = wcwidth((wchar_t)i);
        if (width == -1) {
            return make_error(env, enif_make_atom(env, "not_printable"));
        }
        return enif_make_int(env, width);
#else
        return make_enotsup(env);
#endif
    }
    return enif_make_badarg(env);
}
static ERL_NIF_TERM wcswidth_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    ErlNifBinary bin;
    if (enif_inspect_iolist_as_binary(env, argv[0], &bin)) {
        wchar_t *chars = (wchar_t*)bin.data;
        int width;
#ifdef DEBUG
        for (int i = 0; i < bin.size / sizeof(wchar_t); i++) {
            ASSERT(chars[i] >= 0 && chars[i] < (1l << 21));
        }
#endif
#ifndef __WIN32__
        width = wcswidth(chars, bin.size / sizeof(wchar_t));
#else
        width = bin.size / sizeof(wchar_t);
#endif
        if (width == -1) {
            return make_error(env, enif_make_atom(env, "not_printable"));
        }
        return enif_make_tuple2(env, atom_ok, enif_make_int(env, width));
    }
    return enif_make_badarg(env);
}
static ERL_NIF_TERM sizeof_wchar_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    return enif_make_int(env, sizeof(wchar_t));
}
static ERL_NIF_TERM tty_write_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    ERL_NIF_TERM head = argv[1], tail;
    ErlNifIOQueue *q = NULL;
    ErlNifIOVec vec, *iovec = &vec;
    SysIOVec *iov;
    int iovcnt;
    TTYResource *tty;
    ssize_t res = 0;
    size_t size;
    if (!enif_get_resource(env, argv[0], tty_rt, (void **)&tty))
        return enif_make_badarg(env);
    while (!enif_is_identical(head, enif_make_list(env, 0))) {
        if (!enif_inspect_iovec(env, MAXIOV, head, &tail, &iovec))
            return enif_make_badarg(env);
        head = tail;
        iov = iovec->iov;
        size = iovec->size;
        iovcnt = iovec->iovcnt;
        do {
#ifndef __WIN32__
            do {
                res = writev(tty->ofd, iov, iovcnt);
            } while(res < 0 && (errno == EINTR || errno == EAGAIN));
#else
            res = 0;
            for (int i = 0; i < iovcnt; i++) {
                ssize_t written;
#ifdef HARD_DEBUG
		for (int y = 0; y < iov[i].iov_len; y++)
                    debug("Write %u\r\n",iov[i].iov_base[y]);
#endif
                BOOL r = WriteFile(tty->ofd, iov[i].iov_base,
                                   iov[i].iov_len, &written, NULL);
                if (!r) {
                    res = -1;
                    break;
                }
                res += written;
#ifdef DEBUG
                break;
#endif
            }
#endif
            if (res < 0) {
                if (q) enif_ioq_destroy(q);
                return make_error(env, make_errno(env));
            }
            if (res != size) {
                if (!q) {
                    q = enif_ioq_create(ERL_NIF_IOQ_NORMAL);
                    enif_ioq_enqv(q, iovec, 0);
                }
            }
            if (q) {
                enif_ioq_deq(q, res, &size);
                if (size == 0) {
                    enif_ioq_destroy(q);
                    q = NULL;
                } else {
                    iov = enif_ioq_peek(q, &iovcnt);
                }
            }
        } while(q);
    };
    return atom_ok;
}
static ERL_NIF_TERM tty_read_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    TTYResource *tty;
    ErlNifBinary bin;
    ERL_NIF_TERM res_term;
    Uint64 n;
    ssize_t res = 0;
#ifdef __WIN32__
    HANDLE select_event;
#else
    int select_event;
#endif
    ASSERT(argc == 3);
    if (!enif_get_resource(env, argv[0], tty_rt, (void **)&tty))
        return enif_make_badarg(env);
    if (!enif_get_uint64(env, argv[2], &n))
        return enif_make_badarg(env);
    n = n > 1024 ? 1024 : n;
    select_event = tty->ifd;
    debug("tty_read_nif(%T, %T, %T)\r\n",argv[0],argv[1],argv[2]);
#ifdef __WIN32__
    if (GetFileType(tty->ifd) == FILE_TYPE_CHAR && tty->tty == enabled) {
        ssize_t inputs_read, num_characters = 0;
        wchar_t *characters = NULL;
        INPUT_RECORD inputs[128];
        n = MIN(n, sizeof(inputs) / sizeof(inputs[0]));
        ASSERT(tty->tty == enabled);
        if (!ReadConsoleInputW(tty->ifd, inputs, n, &inputs_read)) {
            return make_errno_error(env, "ReadConsoleInput");
        }
        for (int i = 0; i < inputs_read; i++) {
            if (inputs[i].EventType == KEY_EVENT) {
                if (inputs[i].Event.KeyEvent.bKeyDown) {
                    if (inputs[i].Event.KeyEvent.uChar.UnicodeChar != 0) {
                        num_characters++;
                    } else if (i + 1 < inputs_read && !inputs[i+1].Event.KeyEvent.bKeyDown) {
                        num_characters++;
                    }
                }
            }
        }
        enif_alloc_binary(num_characters * sizeof(wchar_t), &bin);
        characters = (wchar_t*)bin.data;
        for (int i = 0; i < inputs_read; i++) {
            switch (inputs[i].EventType)
            {
            case KEY_EVENT:
                if (inputs[i].Event.KeyEvent.bKeyDown) {
                    if (inputs[i].Event.KeyEvent.uChar.UnicodeChar != 0) {
                        debug("Read %u\r\n",inputs[i].Event.KeyEvent.uChar.UnicodeChar);
                        characters[res++] = inputs[i].Event.KeyEvent.uChar.UnicodeChar;
                    } else if (i + 1 < inputs_read && !inputs[i+1].Event.KeyEvent.bKeyDown) {
                        debug("Read %u\r\n",inputs[i+1].Event.KeyEvent.uChar.UnicodeChar);
                        characters[res++] = inputs[i+1].Event.KeyEvent.uChar.UnicodeChar;
                    }
                }
                break;
            case WINDOW_BUFFER_SIZE_EVENT:
                enif_send(env, &tty->self, NULL,
                    enif_make_tuple2(env, argv[1],
                        enif_make_tuple2(env,
                            enif_make_atom(env, "signal"),
                            enif_make_atom(env, "resize"))));
                break;
            case MOUSE_EVENT:
                break;
            case MENU_EVENT:
            case FOCUS_EVENT:
                break;
            default:
                fprintf(stderr,"Unknown event: %d\r\n", inputs[i].EventType);
                break;
            }
        }
        res *= sizeof(wchar_t);
    } else {
        DWORD bytesTransferred;
        BOOL readRes;
        const char *errorFunction;
        if (GetFileType(tty->ifd) == FILE_TYPE_CHAR) {
            enif_alloc_binary(n * sizeof(wchar_t), &bin);
            readRes = ReadConsoleW(tty->ifd, bin.data, n, &bytesTransferred, NULL);
            bytesTransferred *= sizeof(wchar_t);
            errorFunction = "ReadConsoleW";
        }
        else {
            enif_alloc_binary(n, &bin);
            readRes = ReadFile(tty->ifd, bin.data, bin.size, &bytesTransferred, NULL);
            errorFunction = "ReadFile";
        }
        if (readRes) {
            res = bytesTransferred;
            if (res == 0) {
                enif_release_binary(&bin);
                return make_error(env, enif_make_atom(env, "closed"));
            }
        } else {
            DWORD error = GetLastError();
            enif_release_binary(&bin);
            if (error == ERROR_BROKEN_PIPE)
                return make_error(env, enif_make_atom(env, "closed"));
            return make_errno_error(env, errorFunction);
        }
    }
#else
    enif_alloc_binary(n, &bin);
    res = read(tty->ifd, bin.data, bin.size);
    if (res < 0) {
        if (errno != EAGAIN && errno != EINTR) {
            enif_release_binary(&bin);
            return make_errno_error(env, "read");
        }
        res = 0;
    } else if (res == 0) {
        enif_release_binary(&bin);
        return make_error(env, enif_make_atom(env, "closed"));
    }
#endif
    debug("select on %d\r\n",select_event);
    enif_select(env, select_event, ERL_NIF_SELECT_READ, tty, NULL, argv[1]);
    if (res == bin.size) {
	res_term = enif_make_binary(env, &bin);
    } else if (res < bin.size / 2) {
        unsigned char *buff = enif_make_new_binary(env, res, &res_term);
        if (res > 0) {
            memcpy(buff, bin.data, res);
        }
        enif_release_binary(&bin);
    } else {
        enif_realloc_binary(&bin, res);
        res_term = enif_make_binary(env, &bin);
    }
    return enif_make_tuple2(env, atom_ok, res_term);
}
static ERL_NIF_TERM tty_is_open(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    TTYResource *tty;
#ifdef __WIN32__
    HANDLE handle;
#else
    int fd;
#endif
    if (!enif_get_resource(env, argv[0], tty_rt, (void **)&tty))
        return enif_make_badarg(env);
#ifdef __WIN32__
    handle = tty_get_handle(env, argv[1]);
    if (handle != INVALID_HANDLE_VALUE) {
        DWORD bytesAvailable = 0;
        switch (GetFileType(handle))  {
            case FILE_TYPE_CHAR: {
                DWORD eventsAvailable;
                if (!GetNumberOfConsoleInputEvents(handle, &eventsAvailable)) {
                    return atom_false;
                }
                return atom_true;
            }
            case FILE_TYPE_DISK: {
                return atom_true;
            }
            default: {
                DWORD bytesAvailable = 0;
                if (!PeekNamedPipe(handle, NULL, 0, NULL, &bytesAvailable, NULL)) {
                    DWORD err = GetLastError();
                    if (err == ERROR_BROKEN_PIPE) {
                        return atom_false;
                    }
                    else {
                        return make_errno_error(env, "PeekNamedPipe");
                    }
                }
                return atom_true;
            }
        }
    }
#else
    if (tty_get_fd(env, argv[1], &fd)) {
        struct pollfd fds[1];
        int ret;
        fds[0].fd = fd;
        fds[0].events = POLLHUP;
        fds[0].revents = 0;
        ret = poll(fds, 1, 0);
        if (ret < 0) {
            return make_errno_error(env, __FUNCTION__);
        } else if (ret == 0) {
            return atom_true;
        } else if (ret == 1 && fds[0].revents & POLLHUP) {
            return atom_false;
        }
    }
#endif
    return enif_make_badarg(env);
}
static ERL_NIF_TERM setlocale_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
#ifdef __WIN32__
    TTYResource *tty;
    if (!enif_get_resource(env, argv[0], tty_rt, (void **)&tty))
        return enif_make_badarg(env);
    if (tty->dwOutMode)
    {
        if (!SetConsoleOutputCP(CP_UTF8)) {
            return make_errno_error(env, "SetConsoleOutputCP");
        }
    }
    return atom_true;
#elif defined(PRIMITIVE_UTF8_CHECK)
    setlocale(LC_CTYPE, "");
    return enif_make_atom(env, "primitive");
#else
    char *l = setlocale(LC_CTYPE, "");
    if (l != NULL) {
	if (strcmp(nl_langinfo(CODESET), "UTF-8") == 0)
            return atom_true;
    }
    return atom_false;
#endif
}
#ifdef HAVE_TERMCAP
static TERMINAL *saved_term = NULL;
#endif
static ERL_NIF_TERM tty_setupterm_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
#ifdef HAVE_TERMCAP
    int errret;
    if (setupterm(NULL, -1, &errret) < 0) {
        return make_errno_error(env, "setupterm");
    }
    if (saved_term) {
        del_curterm(saved_term);
    }
    saved_term = cur_term;
    return atom_ok;
#else
    return make_enotsup(env);
#endif
}
#ifdef HAVE_TERMCAP
static ERL_NIF_TERM tty_tinfo_make_map(ErlNifEnv* env,
                                       NCURSES_CONST char * const* names,
                                       NCURSES_CONST char * const* codes,
                                       NCURSES_CONST char * const* fnames) {
    ERL_NIF_TERM res = enif_make_list(env, 0);
    ERL_NIF_TERM ks[3] = {
        enif_make_atom(env, "name"),
        enif_make_atom(env, "code"),
        enif_make_atom(env, "full_name")
    };
    for (int i = 0; names[i] && codes[i] && fnames[i]; i++) {
        ERL_NIF_TERM map;
        ERL_NIF_TERM vs[3] = {
            enif_make_string(env, names[i], ERL_NIF_LATIN1),
            enif_make_string(env, codes[i], ERL_NIF_LATIN1),
            enif_make_string(env, fnames[i], ERL_NIF_LATIN1)
        };
         enif_make_map_from_arrays(env, ks, vs, 3, &map);
        res = enif_make_list_cell(env, map, res);
    }
    return res;
}
#endif
static ERL_NIF_TERM tty_tinfo_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
#ifdef HAVE_TERMCAP
    ERL_NIF_TERM ks[3] = {
        enif_make_atom(env, "bool"),
        enif_make_atom(env, "num"),
        enif_make_atom(env, "str")
    };
    ERL_NIF_TERM vs[3] = {
        tty_tinfo_make_map(env, (const char * const*)boolnames, (const char * const*)boolcodes, (const char * const*)boolfnames),
        tty_tinfo_make_map(env, (const char * const*)numnames, (const char * const*)numcodes, (const char * const*)numfnames),
        tty_tinfo_make_map(env, (const char * const*)strnames, (const char * const*)strcodes, (const char * const*)strfnames)
    };
    ERL_NIF_TERM res;
    enif_make_map_from_arrays(env, ks, vs, 3, &res);
    return res;
#else
    return make_enotsup(env);
#endif
}
static ERL_NIF_TERM tty_tigetnum_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
#ifdef HAVE_TERMCAP
    ErlNifBinary CAP;
    if (!enif_inspect_iolist_as_binary(env, argv[0], &CAP))
        return enif_make_badarg(env);
    return enif_make_int(env, tgetnum((char*)CAP.data));
#else
    return make_enotsup(env);
#endif
}
static ERL_NIF_TERM tty_tigetflag_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
#ifdef HAVE_TERMCAP
    ErlNifBinary CAP;
    if (!enif_inspect_iolist_as_binary(env, argv[0], &CAP))
        return enif_make_badarg(env);
    if (tgetflag((char*)CAP.data))
        return atom_true;
    return atom_false;
#else
    return make_enotsup(env);
#endif
}
static ERL_NIF_TERM tty_tigetstr_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
#ifdef HAVE_TERMCAP
    ErlNifBinary CAP, ret;
    char *str;
    if (!enif_inspect_iolist_as_binary(env, argv[0], &CAP))
        return enif_make_badarg(env);
    str = tigetstr((char*)CAP.data);
    if (!str) return atom_false;
    if (str == (char*)-1) return enif_make_badarg(env);
    enif_alloc_binary(strlen(str), &ret);
    memcpy(ret.data, str, strlen(str));
    return enif_make_tuple2(
        env, atom_ok, enif_make_binary(env, &ret));
#else
    return make_enotsup(env);
#endif
}
static int library_refc = 0;
#ifdef HAVE_TERMCAP
static ErlNifMutex *tputs_mutex;
static int tputs_buffer_index;
static int tputs_buffer_size;
#ifdef DEBUG
static unsigned char static_tputs_buffer[2];
#else
static unsigned char static_tputs_buffer[1024];
#endif
static unsigned char *tputs_buffer;
#if defined(__sun) && defined(__SVR4)
static int tty_puts_putc(char c) {
#else
static int tty_puts_putc(int c) {
#endif
    if (tputs_buffer_index == tputs_buffer_size) {
        if (tputs_buffer == static_tputs_buffer) {
            tputs_buffer = enif_alloc(tputs_buffer_size * 2);
            memcpy(tputs_buffer, static_tputs_buffer, tputs_buffer_size);
            tputs_buffer_size *= 2;
        } else {
            tputs_buffer = enif_realloc(tputs_buffer, tputs_buffer_size * 2);
            tputs_buffer_size *= 2;
        }
    }
    tputs_buffer[tputs_buffer_index++] = (unsigned char)c;
    return 0;
}
#endif
static ERL_NIF_TERM tty_tputs_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
#ifdef HAVE_TERMCAP
    ErlNifBinary cap;
    ERL_NIF_TERM ret;
    char *ent;
    unsigned char *buff;
    long params[9] = { 0 };
    int slot = 0;
    if (!enif_inspect_iolist_as_binary(env, argv[0], &cap))
        return enif_make_badarg(env);
    {
        ERL_NIF_TERM head, tail = argv[1];
        while(enif_get_list_cell(env, tail, &head, &tail)) {
            if (!enif_get_long(env, head, params + slot)) {
                return enif_make_badarg(env);
            }
            slot++;
        }
        if (!enif_is_empty_list(env, tail)) {
            enif_make_badarg(env);
        }
    }
    enif_mutex_lock(tputs_mutex);
    if (slot) {
        ent = tparm((char*)cap.data, params[0], params[1], params[2], params[3],
                    params[4], params[5], params[6], params[7], params[8]);
        if (!ent) {
            enif_mutex_unlock(tputs_mutex);
            return make_errno_error(env, "tparm");
        }
    } else {
        ent = (char*)cap.data;
    }
    tputs_buffer_index = 0;
    tputs_buffer_size = sizeof(static_tputs_buffer);
    tputs_buffer = static_tputs_buffer;
    (void)tputs(ent, 1, tty_puts_putc);
    buff = enif_make_new_binary(env, tputs_buffer_index, &ret);
    memcpy(buff, tputs_buffer, tputs_buffer_index);
    if (tputs_buffer != static_tputs_buffer) {
        enif_free(tputs_buffer);
    }
    enif_mutex_unlock(tputs_mutex);
    return enif_make_tuple2(env, atom_ok, ret);
#else
    return make_enotsup(env);
#endif
}
static ERL_NIF_TERM tty_create_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    TTYResource *tty = enif_alloc_resource(tty_rt, sizeof(TTYResource));
    ERL_NIF_TERM tty_term;
    memset(tty, 0, sizeof(*tty));
#ifdef HARD_DEBUG
    logFile = fopen("tty.log","w+");
#endif
    tty->tty = unavailable;
#ifndef __WIN32__
    tty->ifd = 0;
    tty->ofd = 0;
    if (!tty_get_fd(env, argv[0], &tty->ofd) || tty->ofd == 0)
        return enif_make_badarg(env);
#ifdef HAVE_TERMCAP
    if (tcgetattr(tty->ofd, &tty->tty_rmode) >= 0) {
        tty->tty = disabled;
    }
    tty->tty_smode = tty->tty_rmode;
#endif
#else
    if (enif_is_identical(argv[0], atom_stdin))
        return enif_make_badarg(env);
    if (enif_is_identical(argv[0], atom_stdout)) {
        tty->ifd = GetStdHandle(STD_INPUT_HANDLE);
        if (tty->ifd == INVALID_HANDLE_VALUE || tty->ifd == NULL) {
            tty->ifd = CreateFile("nul", GENERIC_READ, 0,
                                  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        }
    } else {
        tty->ifd = INVALID_HANDLE_VALUE;
    }
    tty->ofd = tty_get_handle(env, argv[0]);
    if (tty->ofd == INVALID_HANDLE_VALUE || tty->ofd == NULL) {
        tty->ofd = CreateFile("nul", GENERIC_WRITE, 0,
                              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (GetConsoleMode(tty->ofd, &tty->dwOriginalOutMode))
    {
        tty->dwOutMode = ENABLE_VIRTUAL_TERMINAL_PROCESSING | tty->dwOriginalOutMode;
        if (!SetConsoleMode(tty->ofd, tty->dwOutMode)) {
            return make_errno_error(env, "SetConsoleModeOut");
        }
        tty->tty = disabled;
    }
    if (GetConsoleMode(tty->ifd, &tty->dwOriginalInMode))
    {
        tty->dwInMode = ENABLE_VIRTUAL_TERMINAL_INPUT | tty->dwOriginalInMode;
        if (!SetConsoleMode(tty->ifd, tty->dwInMode)) {
            return make_errno_error(env, "SetConsoleModeIn");
        }
    } else {
        tty->tty = unavailable;
    }
#endif
    tty_term = enif_make_resource(env, tty);
    enif_release_resource(tty);
    enif_set_pid_undefined(&tty->self);
    enif_set_pid_undefined(&tty->reader);
    return enif_make_tuple2(env, atom_ok, tty_term);
}
static ERL_NIF_TERM tty_init_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    ERL_NIF_TERM input;
    TTYResource *tty;
    debug("tty_init_nif(%T,%T)\r\n", argv[0], argv[1]);
    if (argc != 2 || !enif_is_map(env, argv[1])) {
        return enif_make_badarg(env);
    }
    if (!enif_get_resource(env, argv[0], tty_rt, (void **)&tty))
        return enif_make_badarg(env);
    if (!enif_get_map_value(env, argv[1], enif_make_atom(env, "input"), &input))
        return enif_make_badarg(env);
    if (tty->tty == unavailable) {
        if (enif_is_identical(input, atom_raw))
            return make_enotsup(env);
        return atom_ok;
    }
#if defined(HAVE_TERMCAP) || defined(__WIN32__)
    tty->tty = enif_is_identical(input, atom_raw) ? enabled : disabled;
#ifndef __WIN32__
    if (tty->tty == enabled || sys_memcmp(&tty->tty_smode, &tty->tty_rmode, sizeof(tty->tty_rmode)) != 0) {
        tty->tty_smode = tty->tty_rmode;
        if (tty->tty == enabled) {
            tty->tty_smode.c_iflag &= ~ISTRIP;
            tty->tty_smode.c_iflag &= ~ICRNL;
            tty->tty_smode.c_lflag &= ~ICANON;
            tty->tty_smode.c_oflag &= ~OPOST;
            tty->tty_smode.c_cc[VMIN] = 1;
            tty->tty_smode.c_cc[VTIME] = 0;
    #ifdef VDSUSP
            tty->tty_smode.c_cc[VDSUSP] = 0;
    #endif
            tty->tty_smode.c_lflag &= ~ECHO;
        }
        if (tcsetattr(tty->ofd, TCSANOW, &tty->tty_smode) < 0) {
            return make_errno_error(env, "tcsetattr");
        }
    }
#else
    DWORD dwOutMode = tty->dwOutMode;
    DWORD dwInMode = tty->dwInMode;
    debug("origOutMode: %x origInMode: %x\r\n",
          tty->dwOriginalOutMode, tty->dwOriginalInMode);
    if (tty->tty == enabled) {
        dwOutMode  |= DISABLE_NEWLINE_AUTO_RETURN;
        dwInMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
    }
    if (tty->ifd != INVALID_HANDLE_VALUE && !SetConsoleMode(tty->ifd, dwInMode))
    {
        return make_errno_error(env, "SetConsoleModeInitIn");
    }
    if (!SetConsoleMode(tty->ofd, dwOutMode)) {
        ;
    }
#endif
    enif_self(env, &tty->self);
    enif_monitor_process(env, tty, &tty->self, NULL);
    return atom_ok;
#else
    return make_enotsup(env);
#endif
}
static ERL_NIF_TERM tty_window_size_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    TTYResource *tty;
    int width = -1, height = -1;
    if (!enif_get_resource(env, argv[0], tty_rt, (void **)&tty))
        return enif_make_badarg(env);
    {
#ifdef TIOCGWINSZ
        struct winsize ws;
        if (ioctl(tty->ifd,TIOCGWINSZ,&ws) == 0) {
            if (ws.ws_col > 0)
                width = ws.ws_col;
            if (ws.ws_row > 0)
                height = ws.ws_row;
        } else if (ioctl(tty->ofd,TIOCGWINSZ,&ws) == 0) {
            if (ws.ws_col > 0)
                width = ws.ws_col;
            if (ws.ws_row > 0)
                height = ws.ws_row;
        }
#elif defined(__WIN32__)
        CONSOLE_SCREEN_BUFFER_INFOEX buffer_info;
        buffer_info.cbSize = sizeof(buffer_info);
        if (GetConsoleScreenBufferInfoEx(tty->ofd, &buffer_info)) {
            height = buffer_info.dwSize.Y;
            width = buffer_info.dwSize.X;
        } else {
            return make_errno_error(env,"GetConsoleScreenBufferInfoEx");
        }
#endif
    }
    if (width == -1 && height == -1) {
        return make_enotsup(env);
    }
    return enif_make_tuple2(
        env, atom_ok,
        enif_make_tuple2(
            env,
            enif_make_int(env, width),
            enif_make_int(env, height)
            ));
}
static ERL_NIF_TERM tty_select_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    TTYResource *tty;
#ifndef __WIN32__
    extern int using_oldshell;
#endif
    if (!enif_get_resource(env, argv[0], tty_rt, (void **)&tty))
        return enif_make_badarg(env);
#ifndef __WIN32__
    using_oldshell = 0;
#endif
    debug("Select on %d\r\n", tty->ifd);
    enif_select(env, tty->ifd, ERL_NIF_SELECT_READ, tty, NULL, argv[1]);
    enif_self(env, &tty->reader);
    enif_monitor_process(env, tty, &tty->reader, NULL);
    return atom_ok;
}
static void tty_monitor_down(ErlNifEnv* caller_env, void* obj, ErlNifPid* pid, ErlNifMonitor* mon) {
    TTYResource *tty = obj;
#ifdef HAVE_TERMCAP
    if (enif_compare_pids(pid, &tty->self) == 0) {
        tcsetattr(tty->ifd, TCSANOW, &tty->tty_rmode);
    }
#endif
    if (enif_compare_pids(pid, &tty->reader) == 0) {
        enif_select(caller_env, tty->ifd, ERL_NIF_SELECT_STOP, tty, NULL, atom_undefined);
    }
}
static void tty_select_stop(ErlNifEnv* caller_env, void* obj, ErlNifEvent event, int is_direct_call) {
#ifndef __WIN32__
    if (event != 0)
        close(event);
#endif
}
static void init(ErlNifEnv* env, ErlNifResourceFlags rt_flags) {
    ErlNifResourceTypeInit rt = {
        NULL ,
        tty_select_stop,
        tty_monitor_down};
    tty_rt = enif_open_resource_type_x(env, "tty", &rt, rt_flags, NULL);
    if (library_refc == 0) {
#ifdef HAVE_TERMCAP
        tputs_mutex = enif_mutex_create("tputs_muex");
#endif
#define ATOM_DECL(A) atom_##A = enif_make_atom(env, #A)
        ATOMS
#undef ATOM_DECL
    }
    ++library_refc;
}
static int load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
{
    *priv_data = NULL;
    init(env, ERL_NIF_RT_CREATE);
    return 0;
}
static void unload(ErlNifEnv* env, void* priv_data)
{
    --library_refc;
#ifdef HAVE_TERMCAP
    if (library_refc == 0) {
        enif_mutex_destroy(tputs_mutex);
        tputs_mutex = NULL;
        if (saved_term) {
            del_curterm(saved_term);
            saved_term = NULL;
        }
    }
#endif
}
static int upgrade(ErlNifEnv* env, void** priv_data, void** old_priv_data,
                   ERL_NIF_TERM load_info)
{
    if (*old_priv_data != NULL) {
        return -1;
    }
    if (*priv_data != NULL) {
        return -1;
    }
    *priv_data = NULL;
    init(env, ERL_NIF_RT_TAKEOVER);
    return 0;
}