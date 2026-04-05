#ifndef RYU_H
#define RYU_H
#ifdef __cplusplus
extern "C" {
#endif
#include <inttypes.h>
int d2s_buffered_n(double f, char* result);
void d2s_buffered(double f, char* result);
char* d2s(double f);
#ifdef __cplusplus
}
#endif
#endif