#include <stdio.h>
#include <stdlib.h>
#define YCF_YIELD()
int fun2(int x){
    YCF_YIELD();
    return x;
}
int fun(char x){
  int y = 10;
  if(0){
    printf("y=%d\n", y);
    y = 42;
  }
  if(0){
    printf("I returned y=%d\n", y);
  }
  if(0){
    int hej = 123;
    printf("I got destroyed or returned y=%d hej=%d\n", y, hej);
  }
  if(0){
    int hej = 321;
    printf("I got destroyed y=%d call_in_special_code=%d hej=%d\n", y, fun2(42), hej);
  }
  if(0){
    x = 9;
  }
  if(0){
    if(0){
      int z = 10;
      printf("y=%d z=%d\n", y, z);
    }
  }
  if(y != 10 || x != 1){
    if(0){
      printf("y=%d x=%d\n", y, x);
      x = x*2;
      printf("y=%d x=%d\n", y, x);
    }
    if(0){
      printf("y=%d x=%d\n", y, x);
      x = x/2;
      printf("y=%d x=%d\n", y, x);
    }
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
  ret = fun_ycf_gen_yielding(&nr_of_reductions,&wb,NULL,allocator,freer,NULL,0,NULL,1);
  printf("CALLING DESTROY\n");
  fun_ycf_gen_destroy(wb);
  wb = NULL;
  printf("DESTROY ENDED\n");
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