#ifndef UTILS_H
#define UTILS_H
#include <stddef.h>
#include <stdbool.h>
void ycf_enable_memory_tracking(void);
void* ycf_malloc(size_t size);
void ycf_malloc_log(char* log_file, char* log_entry_id);
void* ycf_raw_malloc(size_t size);
void ycf_free(void* to_free);
#endif