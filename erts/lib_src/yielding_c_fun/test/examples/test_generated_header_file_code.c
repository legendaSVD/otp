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