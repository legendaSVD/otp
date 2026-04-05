#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "sys.h"
#include "global.h"
#include "erl_process.h"
void
erts_sys_init_float(void)
{
# ifdef SIGFPE
    sys_signal(SIGFPE, SIG_IGN);
# endif
}
#define ISDIGIT(d) ((d) >= '0' && (d) <= '9')
int
sys_double_to_chars_ext(double fp, char *buffer, size_t buffer_size, size_t decimals)
{
    char *s = buffer;
    if (erts_snprintf(buffer, buffer_size, "%.*e", decimals, fp) >= buffer_size)
        return -1;
    if (*s == '+' || *s == '-') s++;
    while (ISDIGIT(*s)) s++;
    if (*s == ',') *s++ = '.';
    while (*s) s++;
    return s-buffer;
}
int
sys_chars_to_double(char* buf, double* fp)
{
    char *s = buf, *t, *dp;
    if (*s == '+' || *s == '-') s++;
    if (!ISDIGIT(*s))
      return -1;
    while (ISDIGIT(*s)) s++;
    if (*s != '.' && *s != ',')
      return -1;
    dp = s++;
    if (!ISDIGIT(*s))
      return -1;
    while (ISDIGIT(*s)) s++;
    if (*s == 'e' || *s == 'E') {
	s++;
	if (*s == '+' || *s == '-') s++;
	if (!ISDIGIT(*s))
	  return -1;
	while (ISDIGIT(*s)) s++;
    }
    if (*s)
      return -1;
    errno = 0;
    *fp = strtod(buf, &t);
    if (!erts_isfinite(*fp)) {
        return -1;
    }
    if (t != s) {
	*dp = (*dp == '.') ? ',' : '.';
	errno = 0;
	__ERTS_FP_CHECK_INIT(fpexnp);
	*fp = strtod(buf, &t);
        if (!erts_isfinite(*fp)) {
            return -1;
        }
    }
    if (errno == ERANGE) {
	if (*fp == HUGE_VAL || *fp == -HUGE_VAL) {
	    return -1;
	} else if (t == s && *fp == 0.0) {
	    errno = 0;
	} else if (*fp == 0.0) {
	    return -1;
	}
    }
    return 0;
}
#ifdef USE_MATHERR
int
matherr(struct exception *exc)
{
    return 1;
}
#endif