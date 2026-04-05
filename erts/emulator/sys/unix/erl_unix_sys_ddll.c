#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "sys.h"
#include "erl_vm.h"
#include "global.h"
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#ifndef RTLD_NOW
#  define RTLD_NOW 1
#endif
#define MAX_NAME_LEN 255
#define EXT_LEN      3
#define FILE_EXT     ".so"
static char **errcodes = NULL;
static int num_errcodes = 0;
static int num_errcodes_allocated = 0;
#define my_strdup(WHAT) my_strdup_in(ERTS_ALC_T_DDLL_ERRCODES, WHAT);
static char *my_strdup_in(ErtsAlcType_t type, char *what)
{
    char *res = erts_alloc(type, strlen(what) + 1);
    strcpy(res, what);
    return res;
}
static int find_errcode(char *string, ErtsSysDdllError* err)
{
    int i;
    if (err != NULL) {
	erts_sys_ddll_free_error(err);
	err->str = my_strdup_in(ERTS_ALC_T_DDLL_TMP_BUF, string);
	return 0;
    }
    for(i=0;i<num_errcodes;++i) {
	if (!strcmp(string, errcodes[i])) {
	    return i;
	}
    }
    if (num_errcodes_allocated == num_errcodes) {
	errcodes = (num_errcodes_allocated == 0)
	    ? erts_alloc(ERTS_ALC_T_DDLL_ERRCODES,
			 (num_errcodes_allocated = 10) * sizeof(char *))
	    : erts_realloc(ERTS_ALC_T_DDLL_ERRCODES, errcodes,
			   (num_errcodes_allocated += 10) * sizeof(char *));
    }
    errcodes[num_errcodes++] = my_strdup(string);
    return (num_errcodes - 1);
}
void erl_sys_ddll_init(void) {
#if defined(HAVE_DLOPEN) && defined(ERTS_NEED_DLOPEN_BEFORE_DLERROR)
    void *handle = dlopen("/nonexistinglib", RTLD_NOW);
    if (handle)
	dlclose(handle);
#endif
    return;
}
int erts_sys_ddll_open(const char *full_name, void **handle, ErtsSysDdllError* err)
{
#if defined(HAVE_DLOPEN)
    char* dlname;
    int len = sys_strlen(full_name);
    int ret;
    dlname = erts_alloc(ERTS_ALC_T_TMP, len + EXT_LEN + 1);
    sys_strcpy(dlname, full_name);
    sys_strcpy(dlname+len, FILE_EXT);
    ret = erts_sys_ddll_open_noext(dlname, handle, err);
    erts_free(ERTS_ALC_T_TMP, (void *) dlname);
    return ret;
#else
    return ERL_DE_ERROR_NO_DDLL_FUNCTIONALITY;
#endif
}
int erts_sys_ddll_open_noext(char *dlname, void **handle, ErtsSysDdllError* err)
{
#if defined(HAVE_DLOPEN)
    int ret = ERL_DE_NO_ERROR;
    char *str;
    dlerror();
    if ((*handle = dlopen(dlname, RTLD_NOW | RTLD_LOCAL)) == NULL) {
	str = dlerror();
	if (err == NULL) {
	    if (strstr(str,dlname) == str) {
		char *save_str = str;
		str += strlen(dlname);
		while (*str == ':' || *str == ' ') {
		    ++str;
		}
		if (*str == '\0') {
		    str = save_str;
		}
	    }
	}
	ret = ERL_DE_DYNAMIC_ERROR_OFFSET - find_errcode(str, err);
    }
    return ret;
#else
    return ERL_DE_ERROR_NO_DDLL_FUNCTIONALITY;
#endif
}
int erts_sys_ddll_sym2(void *handle, const char *func_name, void **function,
		       ErtsSysDdllError* err)
{
#if defined(HAVE_DLOPEN)
    void *sym;
    char *e;
    int ret;
    dlerror();
    sym = dlsym(handle, func_name);
    if ((e = dlerror()) != NULL) {
	ret = ERL_DE_DYNAMIC_ERROR_OFFSET - find_errcode(e, err);
        ASSERT(ret != ERL_DE_NO_ERROR);
    } else {
	*function = sym;
	ret = ERL_DE_NO_ERROR;
    }
    return ret;
#else
    return ERL_DE_ERROR_NO_DDLL_FUNCTIONALITY;
#endif
}
int erts_sys_ddll_vsym2(void *handle, const char *func_name, const char *vers,
		       void **function, ErtsSysDdllError* err)
{
#if defined(HAVE_DLVSYM)
    void *sym;
    char *e;
    int ret;
    dlerror();
    sym = dlvsym(handle, func_name, vers);
    if ((e = dlerror()) != NULL) {
	ret = ERL_DE_DYNAMIC_ERROR_OFFSET - find_errcode(e, err);
        ASSERT(ret != ERL_DE_NO_ERROR);
    } else {
	*function = sym;
	ret = ERL_DE_NO_ERROR;
    }
    return ret;
#else
    return ERL_DE_ERROR_NO_DDLL_FUNCTIONALITY;
#endif
}
int erts_sys_ddll_load_driver_init(void *handle, void **function)
{
    void *fn = NULL;
    int res;
    if ((res = erts_sys_ddll_sym2(handle, "driver_init", &fn, NULL)) != ERL_DE_NO_ERROR) {
	res = erts_sys_ddll_sym2(handle, "_driver_init", &fn, NULL);
    }
    if (res == ERL_DE_NO_ERROR) {
        ASSERT(fn);
	*function = fn;
    }
    return res;
}
int erts_sys_ddll_load_nif_init(void *handle, void **function, ErtsSysDdllError* err)
{
    void *fn = NULL;
    int res;
    if ((res = erts_sys_ddll_sym2(handle, "nif_init", &fn, err)) != ERL_DE_NO_ERROR) {
	res = erts_sys_ddll_sym2(handle, "_nif_init", &fn, err);
    }
    if (res == ERL_DE_NO_ERROR) {
        ASSERT(fn);
	*function = fn;
    }
    return res;
}
void *erts_sys_ddll_call_init(void *function) {
    void *(*initfn)(void) = function;
    return (*initfn)();
}
void *erts_sys_ddll_call_nif_init(void *function) {
    return erts_sys_ddll_call_init(function);
}
int erts_sys_ddll_close2(void *handle, ErtsSysDdllError* err)
{
#if defined(HAVE_DLOPEN)
    int ret;
    char *s;
    dlerror();
    if (dlclose(handle) == 0) {
	ret = ERL_DE_NO_ERROR;
    } else {
	if ((s = dlerror()) == NULL) {
	    find_errcode("unspecified error", err);
	    ret = ERL_DE_ERROR_UNSPECIFIED;
	} else {
	    ret = ERL_DE_DYNAMIC_ERROR_OFFSET - find_errcode(s, err);
	}
    }
    return ret;
#else
    return ERL_DE_ERROR_NO_DDLL_FUNCTIONALITY;
#endif
}
char *erts_sys_ddll_error(int code)
{
    int actual_code;
    if (code > ERL_DE_DYNAMIC_ERROR_OFFSET) {
	return "Unspecified error";
    }
    actual_code = -1*(code - ERL_DE_DYNAMIC_ERROR_OFFSET);
#if defined(HAVE_DLOPEN)
    {
	char *msg;
	if (actual_code >= num_errcodes) {
	    msg = "Unknown dlload error";
	} else {
	    msg = errcodes[actual_code];
	}
	return msg;
    }
#endif
    return "no error";
}
void erts_sys_ddll_free_error(ErtsSysDdllError* err)
{
    if (err->str != NULL) {
	erts_free(ERTS_ALC_T_DDLL_TMP_BUF, err->str);
    }
}