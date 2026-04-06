#include <stdio.h>
#include <stdlib.h>
#define YCF_YIELD()
int A(int depth);
int B(int depth);
int A(int depth){
  int b;
  YCF_YIELD();
  depth++;
  printf("A ");
  YCF_YIELD();
  if(depth == 100){
    return 1;
  } else {
    b = B(depth);
  }
  YCF_YIELD();
  return b + 1;
}
int B(int depth){
  int a;
  YCF_YIELD();
  depth++;
  printf("B ");
  YCF_YIELD();
  if(depth == 100){
    YCF_YIELD();
    return 1;
  } else {
    a = A(depth);
  }
  YCF_YIELD();
  return a + 1;
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
    ret = A_ycf_gen_yielding(&nr_of_reductions,&wb,NULL,allocator,freer,NULL,0,NULL,0);
    if(wb != NULL){
      printf("TRAPPED\n");
    }
  }while(wb != NULL);
  if(wb != NULL){
    free(wb);
  }
#else
  ret = A(0);
#endif
  printf("RETURNED %d\n", ret);
  if(ret != A(0)){
    return 1;
  }else{
    return 0;
  }
}