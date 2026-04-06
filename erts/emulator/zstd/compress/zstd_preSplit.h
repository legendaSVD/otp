#ifndef ZSTD_PRESPLIT_H
#define ZSTD_PRESPLIT_H
#include <stddef.h>
#define ZSTD_SLIPBLOCK_WORKSPACESIZE 8208
size_t ZSTD_splitBlock(const void* blockStart, size_t blockSize,
                    int level,
                    void* workspace, size_t wkspSize);
#endif