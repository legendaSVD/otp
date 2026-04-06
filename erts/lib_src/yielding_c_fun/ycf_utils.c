#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "ycf_utils.h"
#include <stdint.h>
bool ycf_track_memory = false;
size_t ycf_memory_usage = 0;
size_t ycf_max_memory_usage = 0;
void ycf_enable_memory_tracking(){
  ycf_track_memory = true;
}
void ycf_malloc_log(char* log_file, char* log_entry_id) {
    FILE* out = fopen(log_file, "a");
    fprintf(out,
            "(%s)\nMax memory consumption %zu bytes ~ %zu kilo bytes ~ %zu mega bytes (after=%zu)\n",
            log_entry_id,
            ycf_max_memory_usage,
            ycf_max_memory_usage / 1000,
            ycf_max_memory_usage / 1000000,
            ycf_memory_usage);
    fclose(out);
}
void* ycf_raw_malloc(size_t size) {
  if (ycf_track_memory) {
    void* block = malloc(size + sizeof(intptr_t));
    intptr_t* size_ptr = block;
    *size_ptr = size + sizeof(intptr_t);
    ycf_memory_usage = ycf_memory_usage + size + sizeof(intptr_t);
    if (ycf_memory_usage > ycf_max_memory_usage) {
      ycf_max_memory_usage = ycf_memory_usage;
    }
    if(block == NULL) {
      fprintf(stderr, "ycf_malloc failed: is there enough memory in the machine?\n");
      exit(1);
    }
    return (void*)(((char*)block) + sizeof(intptr_t));
  } else {
    void* block = malloc(size);
    if(block == NULL) {
      fprintf(stderr, "ycf_malloc failed: is there enough memory in the machine?\n");
      exit(1);
    }
    return block;
  }
}
void* ycf_malloc(size_t size) {
  return ycf_raw_malloc(size);
}
void ycf_free(void* to_free) {
  if (ycf_track_memory) {
    char* to_free_cp = to_free;
    char* start = to_free_cp - sizeof(intptr_t);
    intptr_t* size_ptr = (intptr_t*)start;
    ycf_memory_usage = ycf_memory_usage - *size_ptr;
    free(start);
  } else {
    free(to_free);
  }
}