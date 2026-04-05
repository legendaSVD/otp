#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "sys.h"
#include "signal.h"
void
erts_sys_init_float(void)
{
}
int
sys_chars_to_double(char *buf, double *fp)
{
    unsigned char *s = buf, *t, *dp;
    if (*s == '+' || *s == '-') s++;
    if (!isdigit(*s))
      return -1;
    while (isdigit(*s)) s++;
    if (*s != '.' && *s != ',')
      return -1;
    dp = s++;
    if (!isdigit(*s))
      return -1;
    while (isdigit(*s)) s++;
    if (*s == 'e' || *s == 'E') {
	s++;
	if (*s == '+' || *s == '-') s++;
	if (!isdigit(*s))
	  return -1;
	while (isdigit(*s)) s++;
    }
    if (*s)
      return -1;
    errno = 0;
    *fp = strtod(buf, &t);
    if (t != s) {
	*dp = (*dp == '.') ? ',' : '.';
	errno = 0;
	*fp = strtod(buf, &t);
	if (t != s) {
	    return -1;
	}
    }
    if (*fp < -1.0e-307 || 1.0e-307 < *fp) {
	if (errno == ERANGE) {
	    return -1;
	}
    } else {
	if (errno == ERANGE) {
	    *fp = atof(buf);
	}
    }
    return 0;
}
int
sys_double_to_chars_ext(double fp, char *buffer, size_t buffer_size, size_t decimals)
{
    unsigned char *s = buffer;
    if (erts_snprintf(buffer, buffer_size, "%.*e", decimals, fp) >= buffer_size)
        return -1;
    if (*s == '+' || *s == '-') s++;
    while (isdigit(*s)) s++;
    if (*s == ',') *s++ = '.';
    while (*s) s++;
    return s-buffer;
}
#ifdef USE_MATHERR
int
matherr(struct _exception *exc)
{
    DEBUGF(("FP exception (matherr) (0x%x)\n", exc->type));
    return 1;
}
#endif
static void
fpe_exception(int sig)
{
    erl_fp_exception = 1;
    DEBUGF(("FP exception\n"));
}