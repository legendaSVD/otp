#include <stdio.h>
#include <stdlib.h>
#define YCF_YIELD()
void fun(int level){
  void* my_mem = malloc(1000);
  if(0){
    printf("FREE AT LEVEL %d\n", level);
    free(my_mem);
  }
  if(0){
    printf("I got destroyed or returned %d\n", level);
  }
  printf("LEVEL %d\n", level);
  if (level == 10) {
    YCF_YIELD();
    printf("SHOULD NOT BE PRINTED 1\n");
    return;
  }else {
    fun(level + 1);
    printf("SHOULD NOT BE PRINTED 2\n");
  }
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
  long nr_of_reductions = 1;
  do {
    fun_ycf_gen_yielding(&nr_of_reductions,&wb,NULL,allocator,freer,NULL,0,NULL,1);
    if(wb != NULL){
      printf("TRAPPED\n");
      fun_ycf_gen_destroy(wb);
      printf("DESTROYED\n");
      wb = NULL;
      break;
    }
  } while(wb != NULL);
  if(wb != NULL){
    free(wb);
  }
#else
  fun(1);
#endif
  printf("RETURNED\n");
  return 0;
}