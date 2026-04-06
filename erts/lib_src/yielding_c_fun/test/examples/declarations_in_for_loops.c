#include <stdio.h>
#include <stdlib.h>
#define YCF_YIELD()
int fun(){
  int y = 0;
  for(int x = 0; x < 10; x++){
    y++;
    for(int x = 0; x < 10; x++){
      y++;
    }
  }
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
    nr_of_reductions = 1;
    ret = fun_ycf_gen_yielding(&nr_of_reductions,&wb,NULL,allocator,freer,NULL,0,NULL);
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
  if(ret != 110){
    return 1;
  }else{
    return 0;
  }
}