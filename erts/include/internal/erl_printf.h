#ifndef ERL_PRINTF_H_
#define ERL_PRINTF_H_
#include <stdio.h>
#include <stdarg.h>
extern int (*erts_printf_stdout_func)(char *, va_list);
extern int (*erts_printf_stderr_func)(char *, va_list);
extern int erts_printf_add_cr_to_stdout;
extern int erts_printf_add_cr_to_stderr;
extern int (*erts_printf_block_fpe)(void);
extern void (*erts_printf_unblock_fpe)(int);
typedef struct erts_dsprintf_buf_t_ erts_dsprintf_buf_t;
struct erts_dsprintf_buf_t_ {
    char *str;
    size_t str_len;
    size_t size;
    erts_dsprintf_buf_t *(*grow)(erts_dsprintf_buf_t *, size_t);
};
#define ERTS_DSPRINTF_BUF_INITER(GFUNC) {NULL, 0, 0, (GFUNC)}
typedef int (*fmtfn_t)(void*, char*, size_t);
int erts_write_fd(void *vfdp, char* buf, size_t len);
int erts_write_fp(void *vfdp, char* buf, size_t len);
int erts_write_ds(void *vdsbufp, char* buf, size_t len);
int erts_printf(const char *, ...);
int erts_fprintf(FILE *, const char *, ...);
int erts_fdprintf(int, const char *, ...);
int erts_sprintf(char *, const char *, ...);
int erts_snprintf(char *, size_t, const char *, ...);
int erts_dsprintf(erts_dsprintf_buf_t *, const char *, ...);
int erts_cbprintf(fmtfn_t, void*, const char*, ...);
int erts_vprintf(const char *, va_list);
int erts_vfprintf(FILE *, const char *, va_list);
int erts_vfdprintf(int, const char *, va_list);
int erts_vsprintf(char *, const char *, va_list);
int erts_vsnprintf(char *, size_t, const char *, va_list);
int erts_vdsprintf(erts_dsprintf_buf_t *, const char *, va_list);
int erts_vcbprintf(fmtfn_t, void*, const char*, va_list);
#endif