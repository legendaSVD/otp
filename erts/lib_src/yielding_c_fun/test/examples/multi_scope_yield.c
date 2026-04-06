#include <stdio.h>
#include <stdlib.h>
#define YCF_YIELD()
int fun(char x){
  long y = x + 1;
  YCF_YIELD();
  {
    int x = 10;
    YCF_YIELD();
    y = y + x;
    YCF_YIELD();
    {
      long x = 30;
      YCF_YIELD();
      y = y + x;
      YCF_YIELD();
    }
    YCF_YIELD();
    y = y + x;
  }
  YCF_YIELD();
  y = y + x;
  YCF_YIELD();
  return y;
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
  if(ret != 53){
    return 1;
  }else{
    return 0;
  }
}