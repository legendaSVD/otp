#ifndef YIELDING_C_FUN_HELPERS_H
#define YIELDING_C_FUN_HELPERS_H
#include <stdlib.h>
static void* ycf_alloc(size_t size, void* context){
  (void)context;
  return malloc(size);
}
void ycf_free(void* data, void* context){
  (void)context;
  free(data);
}
#endif