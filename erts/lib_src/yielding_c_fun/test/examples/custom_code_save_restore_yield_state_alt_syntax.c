#include <stdio.h>
#include <stdlib.h>
#define YCF_YIELD()
#ifndef YCF_YIELDING_C_FUN_HELPERS
#define ON_SAVE_YIELD_STATE
#define ON_RESTORE_YIELD_STATE
#define ON_DESTROY_STATE
#define ON_RETURN
#define ON_DESTROY_STATE_OR_RETURN
#define YCF_SPECIAL_CODE_START(PARAM) \
          \
  if(0){
#define YCF_SPECIAL_CODE_END() \
  }                            \
#endif
int fun(char x){
  int y = 10;
  YCF_SPECIAL_CODE_START(ON_SAVE_YIELD_STATE);
  printf("y=%d\n", y);
  y = 42;
  YCF_SPECIAL_CODE_END();
  YCF_SPECIAL_CODE_START(ON_RETURN);
  printf("I returned y=%d\n", y);
  YCF_SPECIAL_CODE_END();
  YCF_SPECIAL_CODE_START(ON_DESTROY_STATE_OR_RETURN);
  printf("I got destroyed or returned y=%d\n", y);
  YCF_SPECIAL_CODE_END();
  YCF_SPECIAL_CODE_START(ON_RESTORE_YIELD_STATE);
  x = 9;
  YCF_SPECIAL_CODE_END();
  if(0){
    YCF_SPECIAL_CODE_START(ON_SAVE_YIELD_STATE); {
    int z = 10;
    printf("y=%d z=%d\n", y, z);
    } YCF_SPECIAL_CODE_END();
  }
  if(y != 10 || x != 1){
    YCF_SPECIAL_CODE_START(ON_RESTORE_YIELD_STATE);
    printf("y=%d x=%d\n", y, x);
    x = x*2;
    printf("y=%d x=%d\n", y, x);
    YCF_SPECIAL_CODE_END();
    YCF_SPECIAL_CODE_START(ON_RESTORE_YIELD_STATE);
    printf("y=%d x=%d\n", y, x);
    x = x/2;
    printf("y=%d x=%d\n", y, x);
    YCF_SPECIAL_CODE_END();
    printf("ERROR BEFORE YIELD\n");
    exit(1);
  }
  YCF_YIELD();
  if(y != 42 || x != 9){
    printf("ERROR AFTER YIELD\n");
    exit(1);
  }
  printf("SUCCESS\n");
  return x;
}
void* allocator(size_t size, void* context){
  (void)context;
  return malloc(size);
}
void freer(void* data, void* context){
  (void)context;
  free(data);
}
int main( int argc, const char* argv[] )
{
#ifdef YCF_YIELD_CODE_GENERATED
  void* wb = NULL;
#endif
  int ret = 0;
  long nr_of_reductions = 1;
#ifdef YCF_YIELD_CODE_GENERATED
  do{
    ret = fun_ycf_gen_yielding(&nr_of_reductions,&wb,NULL,allocator,freer,NULL,0,NULL,1);
    if(wb != NULL){
      printf("TRAPPED\n");
    }
  }while(wb != NULL);
  if(wb != NULL){
    free(wb);
  }
#else
  ret = fun(1);
#endif
  printf("RETURNED %d\n", ret);
  if(ret != 9){
    return 1;
  }else{
    return 0;
  }
}