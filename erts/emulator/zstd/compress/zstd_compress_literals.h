#ifndef ZSTD_COMPRESS_LITERALS_H
#define ZSTD_COMPRESS_LITERALS_H
#include "zstd_compress_internal.h"
size_t ZSTD_noCompressLiterals (void* dst, size_t dstCapacity, const void* src, size_t srcSize);
size_t ZSTD_compressRleLiteralsBlock (void* dst, size_t dstCapacity, const void* src, size_t srcSize);
size_t ZSTD_compressLiterals (void* dst, size_t dstCapacity,
                        const void* src, size_t srcSize,
                              void* entropyWorkspace, size_t entropyWorkspaceSize,
                        const ZSTD_hufCTables_t* prevHuf,
                              ZSTD_hufCTables_t* nextHuf,
                              ZSTD_strategy strategy, int disableLiteralCompression,
                              int suspectUncompressible,
                              int bmi2);
#endif