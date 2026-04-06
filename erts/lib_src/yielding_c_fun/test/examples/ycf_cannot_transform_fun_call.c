#include <stdio.h>
#include <stdlib.h>
#define YCF_YIELD()
void sub_fun(char* x){
  *x = *x + 1;
  *x = *x + 1;
}
int sub_fun2(int x, int y){
  return x+y;
}
int fun(char x){
  int y;
  (void)y;
  x = x + 1;
  sub_fun(((&x)));
  int r = 10 + 10 * (sub_fun2(10, 20));
  if (r) {
      printf("G");
  }
  y = sub_fun2(10, 20);
  YCF_YIELD();
  x = x + 1;
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
  fun(1);
#endif
  printf("RETURNED %d\n", ret);
  if(ret != 5){
    return 1;
  }else{
    return 0;
  }
}