#include <stdio.h>
#include <stdarg.h>
int vsn_printf(char* dst, size_t size, const char* format, va_list args);
int sn_printf(char* dst, size_t size, const char* format, ...);
int strn_cpy(char* dst, size_t size, const char* src);
int strn_cat(char* dst, size_t size, const char* src);
int strn_catf(char* dst, size_t size, const char* format, ...);
char* find_str(const char* haystack, int size, const char* needle);