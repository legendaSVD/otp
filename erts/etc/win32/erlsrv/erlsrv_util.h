#ifndef _ERLSRV_UTIL_H
#define _ERLSRV_UTIL_H
extern wchar_t *service_name;
extern wchar_t *real_service_name;
void log_warning(wchar_t *mess);
void log_error(wchar_t *mess);
void log_info(wchar_t *mess);
wchar_t *envdup(wchar_t *env);
wchar_t *arg_to_env(wchar_t **arg);
wchar_t **env_to_arg(wchar_t *env);
#ifndef NDEBUG
void log_debug(wchar_t *mess);
#else
#define log_debug(mess)
#endif
#endif