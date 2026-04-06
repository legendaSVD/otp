#if defined(_MSC_VER)
#  pragma warning(disable : 4204)
#endif
#include "../common/allocations.h"
#include "../common/zstd_deps.h"
#include "../common/mem.h"
#include "../common/pool.h"
#include "../common/threading.h"
#include "zstd_compress_internal.h"
#include "zstd_ldm.h"
#include "zstdmt_compress.h"
#define ZSTD_RESIZE_SEQPOOL 0
#if defined(DEBUGLEVEL) && (DEBUGLEVEL>=2) \
    && !defined(_MSC_VER) \
    && !defined(__MINGW32__)
#  include <stdio.h>
#  include <unistd.h>
#  include <sys/times.h>
#  define DEBUG_PRINTHEX(l,p,n)                                       \
    do {                                                              \
        unsigned debug_u;                                             \
        for (debug_u=0; debug_u<(n); debug_u++)                       \
            RAWLOG(l, "%02X ", ((const unsigned char*)(p))[debug_u]); \
        RAWLOG(l, " \n");                                             \
    } while (0)
static unsigned long long GetCurrentClockTimeMicroseconds(void)
{
   static clock_t _ticksPerSecond = 0;
   if (_ticksPerSecond <= 0) _ticksPerSecond = sysconf(_SC_CLK_TCK);
   {   struct tms junk; clock_t newTicks = (clock_t) times(&junk);
       return ((((unsigned long long)newTicks)*(1000000))/_ticksPerSecond);
}  }
#define MUTEX_WAIT_TIME_DLEVEL 6
#define ZSTD_PTHREAD_MUTEX_LOCK(mutex)                                                  \
    do {                                                                                \
        if (DEBUGLEVEL >= MUTEX_WAIT_TIME_DLEVEL) {                                     \
            unsigned long long const beforeTime = GetCurrentClockTimeMicroseconds();    \
            ZSTD_pthread_mutex_lock(mutex);                                             \
            {   unsigned long long const afterTime = GetCurrentClockTimeMicroseconds(); \
                unsigned long long const elapsedTime = (afterTime-beforeTime);          \
                if (elapsedTime > 1000) {                                               \
                      \
                    DEBUGLOG(MUTEX_WAIT_TIME_DLEVEL,                                    \
                        "Thread took %llu microseconds to acquire mutex %s \n",         \
                        elapsedTime, #mutex);                                           \
            }   }                                                                       \
        } else {                                                                        \
            ZSTD_pthread_mutex_lock(mutex);                                             \
        }                                                                               \
    } while (0)
#else
#  define ZSTD_PTHREAD_MUTEX_LOCK(m) ZSTD_pthread_mutex_lock(m)
#  define DEBUG_PRINTHEX(l,p,n) do { } while (0)
#endif
typedef struct buffer_s {
    void* start;
    size_t capacity;
} Buffer;
static const Buffer g_nullBuffer = { NULL, 0 };
typedef struct ZSTDMT_bufferPool_s {
    ZSTD_pthread_mutex_t poolMutex;
    size_t bufferSize;
    unsigned totalBuffers;
    unsigned nbBuffers;
    ZSTD_customMem cMem;
    Buffer* buffers;
} ZSTDMT_bufferPool;
static void ZSTDMT_freeBufferPool(ZSTDMT_bufferPool* bufPool)
{
    DEBUGLOG(3, "ZSTDMT_freeBufferPool (address:%08X)", (U32)(size_t)bufPool);
    if (!bufPool) return;
    if (bufPool->buffers) {
        unsigned u;
        for (u=0; u<bufPool->totalBuffers; u++) {
            DEBUGLOG(4, "free buffer %2u (address:%08X)", u, (U32)(size_t)bufPool->buffers[u].start);
            ZSTD_customFree(bufPool->buffers[u].start, bufPool->cMem);
        }
        ZSTD_customFree(bufPool->buffers, bufPool->cMem);
    }
    ZSTD_pthread_mutex_destroy(&bufPool->poolMutex);
    ZSTD_customFree(bufPool, bufPool->cMem);
}
static ZSTDMT_bufferPool* ZSTDMT_createBufferPool(unsigned maxNbBuffers, ZSTD_customMem cMem)
{
    ZSTDMT_bufferPool* const bufPool =
        (ZSTDMT_bufferPool*)ZSTD_customCalloc(sizeof(ZSTDMT_bufferPool), cMem);
    if (bufPool==NULL) return NULL;
    if (ZSTD_pthread_mutex_init(&bufPool->poolMutex, NULL)) {
        ZSTD_customFree(bufPool, cMem);
        return NULL;
    }
    bufPool->buffers = (Buffer*)ZSTD_customCalloc(maxNbBuffers * sizeof(Buffer), cMem);
    if (bufPool->buffers==NULL) {
        ZSTDMT_freeBufferPool(bufPool);
        return NULL;
    }
    bufPool->bufferSize = 64 KB;
    bufPool->totalBuffers = maxNbBuffers;
    bufPool->nbBuffers = 0;
    bufPool->cMem = cMem;
    return bufPool;
}
static size_t ZSTDMT_sizeof_bufferPool(ZSTDMT_bufferPool* bufPool)
{
    size_t const poolSize = sizeof(*bufPool);
    size_t const arraySize = bufPool->totalBuffers * sizeof(Buffer);
    unsigned u;
    size_t totalBufferSize = 0;
    ZSTD_pthread_mutex_lock(&bufPool->poolMutex);
    for (u=0; u<bufPool->totalBuffers; u++)
        totalBufferSize += bufPool->buffers[u].capacity;
    ZSTD_pthread_mutex_unlock(&bufPool->poolMutex);
    return poolSize + arraySize + totalBufferSize;
}
static void ZSTDMT_setBufferSize(ZSTDMT_bufferPool* const bufPool, size_t const bSize)
{
    ZSTD_pthread_mutex_lock(&bufPool->poolMutex);
    DEBUGLOG(4, "ZSTDMT_setBufferSize: bSize = %u", (U32)bSize);
    bufPool->bufferSize = bSize;
    ZSTD_pthread_mutex_unlock(&bufPool->poolMutex);
}
static ZSTDMT_bufferPool* ZSTDMT_expandBufferPool(ZSTDMT_bufferPool* srcBufPool, unsigned maxNbBuffers)
{
    if (srcBufPool==NULL) return NULL;
    if (srcBufPool->totalBuffers >= maxNbBuffers)
        return srcBufPool;
    {   ZSTD_customMem const cMem = srcBufPool->cMem;
        size_t const bSize = srcBufPool->bufferSize;
        ZSTDMT_bufferPool* newBufPool;
        ZSTDMT_freeBufferPool(srcBufPool);
        newBufPool = ZSTDMT_createBufferPool(maxNbBuffers, cMem);
        if (newBufPool==NULL) return newBufPool;
        ZSTDMT_setBufferSize(newBufPool, bSize);
        return newBufPool;
    }
}
static Buffer ZSTDMT_getBuffer(ZSTDMT_bufferPool* bufPool)
{
    size_t const bSize = bufPool->bufferSize;
    DEBUGLOG(5, "ZSTDMT_getBuffer: bSize = %u", (U32)bufPool->bufferSize);
    ZSTD_pthread_mutex_lock(&bufPool->poolMutex);
    if (bufPool->nbBuffers) {
        Buffer const buf = bufPool->buffers[--(bufPool->nbBuffers)];
        size_t const availBufferSize = buf.capacity;
        bufPool->buffers[bufPool->nbBuffers] = g_nullBuffer;
        if ((availBufferSize >= bSize) & ((availBufferSize>>3) <= bSize)) {
            DEBUGLOG(5, "ZSTDMT_getBuffer: provide buffer %u of size %u",
                        bufPool->nbBuffers, (U32)buf.capacity);
            ZSTD_pthread_mutex_unlock(&bufPool->poolMutex);
            return buf;
        }
        DEBUGLOG(5, "ZSTDMT_getBuffer: existing buffer does not meet size conditions => freeing");
        ZSTD_customFree(buf.start, bufPool->cMem);
    }
    ZSTD_pthread_mutex_unlock(&bufPool->poolMutex);
    DEBUGLOG(5, "ZSTDMT_getBuffer: create a new buffer");
    {   Buffer buffer;
        void* const start = ZSTD_customMalloc(bSize, bufPool->cMem);
        buffer.start = start;
        buffer.capacity = (start==NULL) ? 0 : bSize;
        if (start==NULL) {
            DEBUGLOG(5, "ZSTDMT_getBuffer: buffer allocation failure !!");
        } else {
            DEBUGLOG(5, "ZSTDMT_getBuffer: created buffer of size %u", (U32)bSize);
        }
        return buffer;
    }
}
#if ZSTD_RESIZE_SEQPOOL
static Buffer ZSTDMT_resizeBuffer(ZSTDMT_bufferPool* bufPool, Buffer buffer)
{
    size_t const bSize = bufPool->bufferSize;
    if (buffer.capacity < bSize) {
        void* const start = ZSTD_customMalloc(bSize, bufPool->cMem);
        Buffer newBuffer;
        newBuffer.start = start;
        newBuffer.capacity = start == NULL ? 0 : bSize;
        if (start != NULL) {
            assert(newBuffer.capacity >= buffer.capacity);
            ZSTD_memcpy(newBuffer.start, buffer.start, buffer.capacity);
            DEBUGLOG(5, "ZSTDMT_resizeBuffer: created buffer of size %u", (U32)bSize);
            return newBuffer;
        }
        DEBUGLOG(5, "ZSTDMT_resizeBuffer: buffer allocation failure !!");
    }
    return buffer;
}
#endif
static void ZSTDMT_releaseBuffer(ZSTDMT_bufferPool* bufPool, Buffer buf)
{
    DEBUGLOG(5, "ZSTDMT_releaseBuffer");
    if (buf.start == NULL) return;
    ZSTD_pthread_mutex_lock(&bufPool->poolMutex);
    if (bufPool->nbBuffers < bufPool->totalBuffers) {
        bufPool->buffers[bufPool->nbBuffers++] = buf;
        DEBUGLOG(5, "ZSTDMT_releaseBuffer: stored buffer of size %u in slot %u",
                    (U32)buf.capacity, (U32)(bufPool->nbBuffers-1));
        ZSTD_pthread_mutex_unlock(&bufPool->poolMutex);
        return;
    }
    ZSTD_pthread_mutex_unlock(&bufPool->poolMutex);
    DEBUGLOG(5, "ZSTDMT_releaseBuffer: pool capacity reached => freeing ");
    ZSTD_customFree(buf.start, bufPool->cMem);
}
#define BUF_POOL_MAX_NB_BUFFERS(nbWorkers) (2*(nbWorkers) + 3)
#define SEQ_POOL_MAX_NB_BUFFERS(nbWorkers) (nbWorkers)
typedef ZSTDMT_bufferPool ZSTDMT_seqPool;
static size_t ZSTDMT_sizeof_seqPool(ZSTDMT_seqPool* seqPool)
{
    return ZSTDMT_sizeof_bufferPool(seqPool);
}
static RawSeqStore_t bufferToSeq(Buffer buffer)
{
    RawSeqStore_t seq = kNullRawSeqStore;
    seq.seq = (rawSeq*)buffer.start;
    seq.capacity = buffer.capacity / sizeof(rawSeq);
    return seq;
}
static Buffer seqToBuffer(RawSeqStore_t seq)
{
    Buffer buffer;
    buffer.start = seq.seq;
    buffer.capacity = seq.capacity * sizeof(rawSeq);
    return buffer;
}
static RawSeqStore_t ZSTDMT_getSeq(ZSTDMT_seqPool* seqPool)
{
    if (seqPool->bufferSize == 0) {
        return kNullRawSeqStore;
    }
    return bufferToSeq(ZSTDMT_getBuffer(seqPool));
}
#if ZSTD_RESIZE_SEQPOOL
static RawSeqStore_t ZSTDMT_resizeSeq(ZSTDMT_seqPool* seqPool, RawSeqStore_t seq)
{
  return bufferToSeq(ZSTDMT_resizeBuffer(seqPool, seqToBuffer(seq)));
}
#endif
static void ZSTDMT_releaseSeq(ZSTDMT_seqPool* seqPool, RawSeqStore_t seq)
{
  ZSTDMT_releaseBuffer(seqPool, seqToBuffer(seq));
}
static void ZSTDMT_setNbSeq(ZSTDMT_seqPool* const seqPool, size_t const nbSeq)
{
  ZSTDMT_setBufferSize(seqPool, nbSeq * sizeof(rawSeq));
}
static ZSTDMT_seqPool* ZSTDMT_createSeqPool(unsigned nbWorkers, ZSTD_customMem cMem)
{
    ZSTDMT_seqPool* const seqPool = ZSTDMT_createBufferPool(SEQ_POOL_MAX_NB_BUFFERS(nbWorkers), cMem);
    if (seqPool == NULL) return NULL;
    ZSTDMT_setNbSeq(seqPool, 0);
    return seqPool;
}
static void ZSTDMT_freeSeqPool(ZSTDMT_seqPool* seqPool)
{
    ZSTDMT_freeBufferPool(seqPool);
}
static ZSTDMT_seqPool* ZSTDMT_expandSeqPool(ZSTDMT_seqPool* pool, U32 nbWorkers)
{
    return ZSTDMT_expandBufferPool(pool, SEQ_POOL_MAX_NB_BUFFERS(nbWorkers));
}
typedef struct {
    ZSTD_pthread_mutex_t poolMutex;
    int totalCCtx;
    int availCCtx;
    ZSTD_customMem cMem;
    ZSTD_CCtx** cctxs;
} ZSTDMT_CCtxPool;
static void ZSTDMT_freeCCtxPool(ZSTDMT_CCtxPool* pool)
{
    if (!pool) return;
    ZSTD_pthread_mutex_destroy(&pool->poolMutex);
    if (pool->cctxs) {
        int cid;
        for (cid=0; cid<pool->totalCCtx; cid++)
            ZSTD_freeCCtx(pool->cctxs[cid]);
        ZSTD_customFree(pool->cctxs, pool->cMem);
    }
    ZSTD_customFree(pool, pool->cMem);
}
static ZSTDMT_CCtxPool* ZSTDMT_createCCtxPool(int nbWorkers,
                                              ZSTD_customMem cMem)
{
    ZSTDMT_CCtxPool* const cctxPool =
        (ZSTDMT_CCtxPool*) ZSTD_customCalloc(sizeof(ZSTDMT_CCtxPool), cMem);
    assert(nbWorkers > 0);
    if (!cctxPool) return NULL;
    if (ZSTD_pthread_mutex_init(&cctxPool->poolMutex, NULL)) {
        ZSTD_customFree(cctxPool, cMem);
        return NULL;
    }
    cctxPool->totalCCtx = nbWorkers;
    cctxPool->cctxs = (ZSTD_CCtx**)ZSTD_customCalloc(nbWorkers * sizeof(ZSTD_CCtx*), cMem);
    if (!cctxPool->cctxs) {
        ZSTDMT_freeCCtxPool(cctxPool);
        return NULL;
    }
    cctxPool->cMem = cMem;
    cctxPool->cctxs[0] = ZSTD_createCCtx_advanced(cMem);
    if (!cctxPool->cctxs[0]) { ZSTDMT_freeCCtxPool(cctxPool); return NULL; }
    cctxPool->availCCtx = 1;
    DEBUGLOG(3, "cctxPool created, with %u workers", nbWorkers);
    return cctxPool;
}
static ZSTDMT_CCtxPool* ZSTDMT_expandCCtxPool(ZSTDMT_CCtxPool* srcPool,
                                              int nbWorkers)
{
    if (srcPool==NULL) return NULL;
    if (nbWorkers <= srcPool->totalCCtx) return srcPool;
    {   ZSTD_customMem const cMem = srcPool->cMem;
        ZSTDMT_freeCCtxPool(srcPool);
        return ZSTDMT_createCCtxPool(nbWorkers, cMem);
    }
}
static size_t ZSTDMT_sizeof_CCtxPool(ZSTDMT_CCtxPool* cctxPool)
{
    ZSTD_pthread_mutex_lock(&cctxPool->poolMutex);
    {   unsigned const nbWorkers = cctxPool->totalCCtx;
        size_t const poolSize = sizeof(*cctxPool);
        size_t const arraySize = cctxPool->totalCCtx * sizeof(ZSTD_CCtx*);
        size_t totalCCtxSize = 0;
        unsigned u;
        for (u=0; u<nbWorkers; u++) {
            totalCCtxSize += ZSTD_sizeof_CCtx(cctxPool->cctxs[u]);
        }
        ZSTD_pthread_mutex_unlock(&cctxPool->poolMutex);
        assert(nbWorkers > 0);
        return poolSize + arraySize + totalCCtxSize;
    }
}
static ZSTD_CCtx* ZSTDMT_getCCtx(ZSTDMT_CCtxPool* cctxPool)
{
    DEBUGLOG(5, "ZSTDMT_getCCtx");
    ZSTD_pthread_mutex_lock(&cctxPool->poolMutex);
    if (cctxPool->availCCtx) {
        cctxPool->availCCtx--;
        {   ZSTD_CCtx* const cctx = cctxPool->cctxs[cctxPool->availCCtx];
            ZSTD_pthread_mutex_unlock(&cctxPool->poolMutex);
            return cctx;
    }   }
    ZSTD_pthread_mutex_unlock(&cctxPool->poolMutex);
    DEBUGLOG(5, "create one more CCtx");
    return ZSTD_createCCtx_advanced(cctxPool->cMem);
}
static void ZSTDMT_releaseCCtx(ZSTDMT_CCtxPool* pool, ZSTD_CCtx* cctx)
{
    if (cctx==NULL) return;
    ZSTD_pthread_mutex_lock(&pool->poolMutex);
    if (pool->availCCtx < pool->totalCCtx)
        pool->cctxs[pool->availCCtx++] = cctx;
    else {
        DEBUGLOG(4, "CCtx pool overflow : free cctx");
        ZSTD_freeCCtx(cctx);
    }
    ZSTD_pthread_mutex_unlock(&pool->poolMutex);
}
typedef struct {
    void const* start;
    size_t size;
} Range;
typedef struct {
    ZSTD_pthread_mutex_t mutex;
    ZSTD_pthread_cond_t cond;
    ZSTD_CCtx_params params;
    ldmState_t ldmState;
    XXH64_state_t xxhState;
    unsigned nextJobID;
    ZSTD_pthread_mutex_t ldmWindowMutex;
    ZSTD_pthread_cond_t ldmWindowCond;
    ZSTD_window_t ldmWindow;
} SerialState;
static int
ZSTDMT_serialState_reset(SerialState* serialState,
                         ZSTDMT_seqPool* seqPool,
                         ZSTD_CCtx_params params,
                         size_t jobSize,
                         const void* dict, size_t const dictSize,
                         ZSTD_dictContentType_e dictContentType)
{
    if (params.ldmParams.enableLdm == ZSTD_ps_enable) {
        DEBUGLOG(4, "LDM window size = %u KB", (1U << params.cParams.windowLog) >> 10);
        ZSTD_ldm_adjustParameters(&params.ldmParams, &params.cParams);
        assert(params.ldmParams.hashLog >= params.ldmParams.bucketSizeLog);
        assert(params.ldmParams.hashRateLog < 32);
    } else {
        ZSTD_memset(&params.ldmParams, 0, sizeof(params.ldmParams));
    }
    serialState->nextJobID = 0;
    if (params.fParams.checksumFlag)
        XXH64_reset(&serialState->xxhState, 0);
    if (params.ldmParams.enableLdm == ZSTD_ps_enable) {
        ZSTD_customMem cMem = params.customMem;
        unsigned const hashLog = params.ldmParams.hashLog;
        size_t const hashSize = ((size_t)1 << hashLog) * sizeof(ldmEntry_t);
        unsigned const bucketLog =
            params.ldmParams.hashLog - params.ldmParams.bucketSizeLog;
        unsigned const prevBucketLog =
            serialState->params.ldmParams.hashLog -
            serialState->params.ldmParams.bucketSizeLog;
        size_t const numBuckets = (size_t)1 << bucketLog;
        ZSTDMT_setNbSeq(seqPool, ZSTD_ldm_getMaxNbSeq(params.ldmParams, jobSize));
        ZSTD_window_init(&serialState->ldmState.window);
        if (serialState->ldmState.hashTable == NULL || serialState->params.ldmParams.hashLog < hashLog) {
            ZSTD_customFree(serialState->ldmState.hashTable, cMem);
            serialState->ldmState.hashTable = (ldmEntry_t*)ZSTD_customMalloc(hashSize, cMem);
        }
        if (serialState->ldmState.bucketOffsets == NULL || prevBucketLog < bucketLog) {
            ZSTD_customFree(serialState->ldmState.bucketOffsets, cMem);
            serialState->ldmState.bucketOffsets = (BYTE*)ZSTD_customMalloc(numBuckets, cMem);
        }
        if (!serialState->ldmState.hashTable || !serialState->ldmState.bucketOffsets)
            return 1;
        ZSTD_memset(serialState->ldmState.hashTable, 0, hashSize);
        ZSTD_memset(serialState->ldmState.bucketOffsets, 0, numBuckets);
        serialState->ldmState.loadedDictEnd = 0;
        if (dictSize > 0) {
            if (dictContentType == ZSTD_dct_rawContent) {
                BYTE const* const dictEnd = (const BYTE*)dict + dictSize;
                ZSTD_window_update(&serialState->ldmState.window, dict, dictSize,  0);
                ZSTD_ldm_fillHashTable(&serialState->ldmState, (const BYTE*)dict, dictEnd, &params.ldmParams);
                serialState->ldmState.loadedDictEnd = params.forceWindow ? 0 : (U32)(dictEnd - serialState->ldmState.window.base);
            } else {
            }
        }
        serialState->ldmWindow = serialState->ldmState.window;
    }
    serialState->params = params;
    serialState->params.jobSize = (U32)jobSize;
    return 0;
}
static int ZSTDMT_serialState_init(SerialState* serialState)
{
    int initError = 0;
    ZSTD_memset(serialState, 0, sizeof(*serialState));
    initError |= ZSTD_pthread_mutex_init(&serialState->mutex, NULL);
    initError |= ZSTD_pthread_cond_init(&serialState->cond, NULL);
    initError |= ZSTD_pthread_mutex_init(&serialState->ldmWindowMutex, NULL);
    initError |= ZSTD_pthread_cond_init(&serialState->ldmWindowCond, NULL);
    return initError;
}
static void ZSTDMT_serialState_free(SerialState* serialState)
{
    ZSTD_customMem cMem = serialState->params.customMem;
    ZSTD_pthread_mutex_destroy(&serialState->mutex);
    ZSTD_pthread_cond_destroy(&serialState->cond);
    ZSTD_pthread_mutex_destroy(&serialState->ldmWindowMutex);
    ZSTD_pthread_cond_destroy(&serialState->ldmWindowCond);
    ZSTD_customFree(serialState->ldmState.hashTable, cMem);
    ZSTD_customFree(serialState->ldmState.bucketOffsets, cMem);
}
static void
ZSTDMT_serialState_genSequences(SerialState* serialState,
                                RawSeqStore_t* seqStore,
                                Range src, unsigned jobID)
{
    ZSTD_PTHREAD_MUTEX_LOCK(&serialState->mutex);
    while (serialState->nextJobID < jobID) {
        DEBUGLOG(5, "wait for serialState->cond");
        ZSTD_pthread_cond_wait(&serialState->cond, &serialState->mutex);
    }
    if (serialState->nextJobID == jobID) {
        if (serialState->params.ldmParams.enableLdm == ZSTD_ps_enable) {
            size_t error;
            DEBUGLOG(6, "ZSTDMT_serialState_genSequences: LDM update");
            assert(seqStore->seq != NULL && seqStore->pos == 0 &&
                   seqStore->size == 0 && seqStore->capacity > 0);
            assert(src.size <= serialState->params.jobSize);
            ZSTD_window_update(&serialState->ldmState.window, src.start, src.size,  0);
            error = ZSTD_ldm_generateSequences(
                &serialState->ldmState, seqStore,
                &serialState->params.ldmParams, src.start, src.size);
            assert(!ZSTD_isError(error)); (void)error;
            ZSTD_PTHREAD_MUTEX_LOCK(&serialState->ldmWindowMutex);
            serialState->ldmWindow = serialState->ldmState.window;
            ZSTD_pthread_cond_signal(&serialState->ldmWindowCond);
            ZSTD_pthread_mutex_unlock(&serialState->ldmWindowMutex);
        }
        if (serialState->params.fParams.checksumFlag && src.size > 0)
            XXH64_update(&serialState->xxhState, src.start, src.size);
    }
    serialState->nextJobID++;
    ZSTD_pthread_cond_broadcast(&serialState->cond);
    ZSTD_pthread_mutex_unlock(&serialState->mutex);
}
static void
ZSTDMT_serialState_applySequences(const SerialState* serialState,
                                  ZSTD_CCtx* jobCCtx,
                                  const RawSeqStore_t* seqStore)
{
    if (seqStore->size > 0) {
        DEBUGLOG(5, "ZSTDMT_serialState_applySequences: uploading %u external sequences", (unsigned)seqStore->size);
        assert(serialState->params.ldmParams.enableLdm == ZSTD_ps_enable); (void)serialState;
        assert(jobCCtx);
        ZSTD_referenceExternalSequences(jobCCtx, seqStore->seq, seqStore->size);
    }
}
static void ZSTDMT_serialState_ensureFinished(SerialState* serialState,
                                              unsigned jobID, size_t cSize)
{
    ZSTD_PTHREAD_MUTEX_LOCK(&serialState->mutex);
    if (serialState->nextJobID <= jobID) {
        assert(ZSTD_isError(cSize)); (void)cSize;
        DEBUGLOG(5, "Skipping past job %u because of error", jobID);
        serialState->nextJobID = jobID + 1;
        ZSTD_pthread_cond_broadcast(&serialState->cond);
        ZSTD_PTHREAD_MUTEX_LOCK(&serialState->ldmWindowMutex);
        ZSTD_window_clear(&serialState->ldmWindow);
        ZSTD_pthread_cond_signal(&serialState->ldmWindowCond);
        ZSTD_pthread_mutex_unlock(&serialState->ldmWindowMutex);
    }
    ZSTD_pthread_mutex_unlock(&serialState->mutex);
}
static const Range kNullRange = { NULL, 0 };
typedef struct {
    size_t   consumed;
    size_t   cSize;
    ZSTD_pthread_mutex_t job_mutex;
    ZSTD_pthread_cond_t job_cond;
    ZSTDMT_CCtxPool* cctxPool;
    ZSTDMT_bufferPool* bufPool;
    ZSTDMT_seqPool* seqPool;
    SerialState* serial;
    Buffer dstBuff;
    Range prefix;
    Range src;
    unsigned jobID;
    unsigned firstJob;
    unsigned lastJob;
    ZSTD_CCtx_params params;
    const ZSTD_CDict* cdict;
    unsigned long long fullFrameSize;
    size_t   dstFlushed;
    unsigned frameChecksumNeeded;
} ZSTDMT_jobDescription;
#define JOB_ERROR(e)                                \
    do {                                            \
        ZSTD_PTHREAD_MUTEX_LOCK(&job->job_mutex);   \
        job->cSize = e;                             \
        ZSTD_pthread_mutex_unlock(&job->job_mutex); \
        goto _endJob;                               \
    } while (0)
static void ZSTDMT_compressionJob(void* jobDescription)
{
    ZSTDMT_jobDescription* const job = (ZSTDMT_jobDescription*)jobDescription;
    ZSTD_CCtx_params jobParams = job->params;
    ZSTD_CCtx* const cctx = ZSTDMT_getCCtx(job->cctxPool);
    RawSeqStore_t rawSeqStore = ZSTDMT_getSeq(job->seqPool);
    Buffer dstBuff = job->dstBuff;
    size_t lastCBlockSize = 0;
    DEBUGLOG(5, "ZSTDMT_compressionJob: job %u", job->jobID);
    if (cctx==NULL) JOB_ERROR(ERROR(memory_allocation));
    if (dstBuff.start == NULL) {
        dstBuff = ZSTDMT_getBuffer(job->bufPool);
        if (dstBuff.start==NULL) JOB_ERROR(ERROR(memory_allocation));
        job->dstBuff = dstBuff;
    }
    if (jobParams.ldmParams.enableLdm == ZSTD_ps_enable && rawSeqStore.seq == NULL)
        JOB_ERROR(ERROR(memory_allocation));
    if (job->jobID != 0) jobParams.fParams.checksumFlag = 0;
    jobParams.ldmParams.enableLdm = ZSTD_ps_disable;
    jobParams.nbWorkers = 0;
    ZSTDMT_serialState_genSequences(job->serial, &rawSeqStore, job->src, job->jobID);
    if (job->cdict) {
        size_t const initError = ZSTD_compressBegin_advanced_internal(cctx, NULL, 0, ZSTD_dct_auto, ZSTD_dtlm_fast, job->cdict, &jobParams, job->fullFrameSize);
        assert(job->firstJob);
        if (ZSTD_isError(initError)) JOB_ERROR(initError);
    } else {
        U64 const pledgedSrcSize = job->firstJob ? job->fullFrameSize : job->src.size;
        {   size_t const forceWindowError = ZSTD_CCtxParams_setParameter(&jobParams, ZSTD_c_forceMaxWindow, !job->firstJob);
            if (ZSTD_isError(forceWindowError)) JOB_ERROR(forceWindowError);
        }
        if (!job->firstJob) {
            size_t const err = ZSTD_CCtxParams_setParameter(&jobParams, ZSTD_c_deterministicRefPrefix, 0);
            if (ZSTD_isError(err)) JOB_ERROR(err);
        }
        DEBUGLOG(6, "ZSTDMT_compressionJob: job %u: loading prefix of size %zu", job->jobID, job->prefix.size);
        {   size_t const initError = ZSTD_compressBegin_advanced_internal(cctx,
                                        job->prefix.start, job->prefix.size, ZSTD_dct_rawContent,
                                        ZSTD_dtlm_fast,
                                        NULL,
                                        &jobParams, pledgedSrcSize);
            if (ZSTD_isError(initError)) JOB_ERROR(initError);
    }   }
    ZSTDMT_serialState_applySequences(job->serial, cctx, &rawSeqStore);
    if (!job->firstJob) {
        size_t const hSize = ZSTD_compressContinue_public(cctx, dstBuff.start, dstBuff.capacity, job->src.start, 0);
        if (ZSTD_isError(hSize)) JOB_ERROR(hSize);
        DEBUGLOG(5, "ZSTDMT_compressionJob: flush and overwrite %u bytes of frame header (not first job)", (U32)hSize);
        ZSTD_invalidateRepCodes(cctx);
    }
    {   size_t const chunkSize = 4*ZSTD_BLOCKSIZE_MAX;
        int const nbChunks = (int)((job->src.size + (chunkSize-1)) / chunkSize);
        const BYTE* ip = (const BYTE*) job->src.start;
        BYTE* const ostart = (BYTE*)dstBuff.start;
        BYTE* op = ostart;
        BYTE* oend = op + dstBuff.capacity;
        int chunkNb;
        if (sizeof(size_t) > sizeof(int)) assert(job->src.size < ((size_t)INT_MAX) * chunkSize);
        DEBUGLOG(5, "ZSTDMT_compressionJob: compress %u bytes in %i blocks", (U32)job->src.size, nbChunks);
        assert(job->cSize == 0);
        for (chunkNb = 1; chunkNb < nbChunks; chunkNb++) {
            size_t const cSize = ZSTD_compressContinue_public(cctx, op, oend-op, ip, chunkSize);
            if (ZSTD_isError(cSize)) JOB_ERROR(cSize);
            ip += chunkSize;
            op += cSize; assert(op < oend);
            ZSTD_PTHREAD_MUTEX_LOCK(&job->job_mutex);
            job->cSize += cSize;
            job->consumed = chunkSize * chunkNb;
            DEBUGLOG(5, "ZSTDMT_compressionJob: compress new block : cSize==%u bytes (total: %u)",
                        (U32)cSize, (U32)job->cSize);
            ZSTD_pthread_cond_signal(&job->job_cond);
            ZSTD_pthread_mutex_unlock(&job->job_mutex);
        }
        assert(chunkSize > 0);
        assert((chunkSize & (chunkSize - 1)) == 0);
        if ((nbChunks > 0) | job->lastJob  ) {
            size_t const lastBlockSize1 = job->src.size & (chunkSize-1);
            size_t const lastBlockSize = ((lastBlockSize1==0) & (job->src.size>=chunkSize)) ? chunkSize : lastBlockSize1;
            size_t const cSize = (job->lastJob) ?
                 ZSTD_compressEnd_public(cctx, op, oend-op, ip, lastBlockSize) :
                 ZSTD_compressContinue_public(cctx, op, oend-op, ip, lastBlockSize);
            if (ZSTD_isError(cSize)) JOB_ERROR(cSize);
            lastCBlockSize = cSize;
    }   }
    if (!job->firstJob) {
        assert(!ZSTD_window_hasExtDict(cctx->blockState.matchState.window));
    }
    ZSTD_CCtx_trace(cctx, 0);
_endJob:
    ZSTDMT_serialState_ensureFinished(job->serial, job->jobID, job->cSize);
    if (job->prefix.size > 0)
        DEBUGLOG(5, "Finished with prefix: %zx", (size_t)job->prefix.start);
    DEBUGLOG(5, "Finished with source: %zx", (size_t)job->src.start);
    ZSTDMT_releaseSeq(job->seqPool, rawSeqStore);
    ZSTDMT_releaseCCtx(job->cctxPool, cctx);
    ZSTD_PTHREAD_MUTEX_LOCK(&job->job_mutex);
    if (ZSTD_isError(job->cSize)) assert(lastCBlockSize == 0);
    job->cSize += lastCBlockSize;
    job->consumed = job->src.size;
    ZSTD_pthread_cond_signal(&job->job_cond);
    ZSTD_pthread_mutex_unlock(&job->job_mutex);
}
typedef struct {
    Range prefix;
    Buffer buffer;
    size_t filled;
} InBuff_t;
typedef struct {
  BYTE* buffer;
  size_t capacity;
  size_t pos;
} RoundBuff_t;
static const RoundBuff_t kNullRoundBuff = {NULL, 0, 0};
#define RSYNC_LENGTH 32
#define RSYNC_MIN_BLOCK_LOG ZSTD_BLOCKSIZELOG_MAX
#define RSYNC_MIN_BLOCK_SIZE (1<<RSYNC_MIN_BLOCK_LOG)
typedef struct {
  U64 hash;
  U64 hitMask;
  U64 primePower;
} RSyncState_t;
struct ZSTDMT_CCtx_s {
    POOL_ctx* factory;
    ZSTDMT_jobDescription* jobs;
    ZSTDMT_bufferPool* bufPool;
    ZSTDMT_CCtxPool* cctxPool;
    ZSTDMT_seqPool* seqPool;
    ZSTD_CCtx_params params;
    size_t targetSectionSize;
    size_t targetPrefixSize;
    int jobReady;
    InBuff_t inBuff;
    RoundBuff_t roundBuff;
    SerialState serial;
    RSyncState_t rsync;
    unsigned jobIDMask;
    unsigned doneJobID;
    unsigned nextJobID;
    unsigned frameEnded;
    unsigned allJobsCompleted;
    unsigned long long frameContentSize;
    unsigned long long consumed;
    unsigned long long produced;
    ZSTD_customMem cMem;
    ZSTD_CDict* cdictLocal;
    const ZSTD_CDict* cdict;
    unsigned providedFactory: 1;
};
static void ZSTDMT_freeJobsTable(ZSTDMT_jobDescription* jobTable, U32 nbJobs, ZSTD_customMem cMem)
{
    U32 jobNb;
    if (jobTable == NULL) return;
    for (jobNb=0; jobNb<nbJobs; jobNb++) {
        ZSTD_pthread_mutex_destroy(&jobTable[jobNb].job_mutex);
        ZSTD_pthread_cond_destroy(&jobTable[jobNb].job_cond);
    }
    ZSTD_customFree(jobTable, cMem);
}
static ZSTDMT_jobDescription* ZSTDMT_createJobsTable(U32* nbJobsPtr, ZSTD_customMem cMem)
{
    U32 const nbJobsLog2 = ZSTD_highbit32(*nbJobsPtr) + 1;
    U32 const nbJobs = 1 << nbJobsLog2;
    U32 jobNb;
    ZSTDMT_jobDescription* const jobTable = (ZSTDMT_jobDescription*)
                ZSTD_customCalloc(nbJobs * sizeof(ZSTDMT_jobDescription), cMem);
    int initError = 0;
    if (jobTable==NULL) return NULL;
    *nbJobsPtr = nbJobs;
    for (jobNb=0; jobNb<nbJobs; jobNb++) {
        initError |= ZSTD_pthread_mutex_init(&jobTable[jobNb].job_mutex, NULL);
        initError |= ZSTD_pthread_cond_init(&jobTable[jobNb].job_cond, NULL);
    }
    if (initError != 0) {
        ZSTDMT_freeJobsTable(jobTable, nbJobs, cMem);
        return NULL;
    }
    return jobTable;
}
static size_t ZSTDMT_expandJobsTable (ZSTDMT_CCtx* mtctx, U32 nbWorkers) {
    U32 nbJobs = nbWorkers + 2;
    if (nbJobs > mtctx->jobIDMask+1) {
        ZSTDMT_freeJobsTable(mtctx->jobs, mtctx->jobIDMask+1, mtctx->cMem);
        mtctx->jobIDMask = 0;
        mtctx->jobs = ZSTDMT_createJobsTable(&nbJobs, mtctx->cMem);
        if (mtctx->jobs==NULL) return ERROR(memory_allocation);
        assert((nbJobs != 0) && ((nbJobs & (nbJobs - 1)) == 0));
        mtctx->jobIDMask = nbJobs - 1;
    }
    return 0;
}
static size_t ZSTDMT_CCtxParam_setNbWorkers(ZSTD_CCtx_params* params, unsigned nbWorkers)
{
    return ZSTD_CCtxParams_setParameter(params, ZSTD_c_nbWorkers, (int)nbWorkers);
}
MEM_STATIC ZSTDMT_CCtx* ZSTDMT_createCCtx_advanced_internal(unsigned nbWorkers, ZSTD_customMem cMem, ZSTD_threadPool* pool)
{
    ZSTDMT_CCtx* mtctx;
    U32 nbJobs = nbWorkers + 2;
    int initError;
    DEBUGLOG(3, "ZSTDMT_createCCtx_advanced (nbWorkers = %u)", nbWorkers);
    if (nbWorkers < 1) return NULL;
    nbWorkers = MIN(nbWorkers , ZSTDMT_NBWORKERS_MAX);
    if ((cMem.customAlloc!=NULL) ^ (cMem.customFree!=NULL))
        return NULL;
    mtctx = (ZSTDMT_CCtx*) ZSTD_customCalloc(sizeof(ZSTDMT_CCtx), cMem);
    if (!mtctx) return NULL;
    ZSTDMT_CCtxParam_setNbWorkers(&mtctx->params, nbWorkers);
    mtctx->cMem = cMem;
    mtctx->allJobsCompleted = 1;
    if (pool != NULL) {
      mtctx->factory = pool;
      mtctx->providedFactory = 1;
    }
    else {
      mtctx->factory = POOL_create_advanced(nbWorkers, 0, cMem);
      mtctx->providedFactory = 0;
    }
    mtctx->jobs = ZSTDMT_createJobsTable(&nbJobs, cMem);
    assert(nbJobs > 0); assert((nbJobs & (nbJobs - 1)) == 0);
    mtctx->jobIDMask = nbJobs - 1;
    mtctx->bufPool = ZSTDMT_createBufferPool(BUF_POOL_MAX_NB_BUFFERS(nbWorkers), cMem);
    mtctx->cctxPool = ZSTDMT_createCCtxPool(nbWorkers, cMem);
    mtctx->seqPool = ZSTDMT_createSeqPool(nbWorkers, cMem);
    initError = ZSTDMT_serialState_init(&mtctx->serial);
    mtctx->roundBuff = kNullRoundBuff;
    if (!mtctx->factory | !mtctx->jobs | !mtctx->bufPool | !mtctx->cctxPool | !mtctx->seqPool | initError) {
        ZSTDMT_freeCCtx(mtctx);
        return NULL;
    }
    DEBUGLOG(3, "mt_cctx created, for %u threads", nbWorkers);
    return mtctx;
}
ZSTDMT_CCtx* ZSTDMT_createCCtx_advanced(unsigned nbWorkers, ZSTD_customMem cMem, ZSTD_threadPool* pool)
{
#ifdef ZSTD_MULTITHREAD
    return ZSTDMT_createCCtx_advanced_internal(nbWorkers, cMem, pool);
#else
    (void)nbWorkers;
    (void)cMem;
    (void)pool;
    return NULL;
#endif
}
static void ZSTDMT_releaseAllJobResources(ZSTDMT_CCtx* mtctx)
{
    unsigned jobID;
    DEBUGLOG(3, "ZSTDMT_releaseAllJobResources");
    for (jobID=0; jobID <= mtctx->jobIDMask; jobID++) {
        ZSTD_pthread_mutex_t const mutex = mtctx->jobs[jobID].job_mutex;
        ZSTD_pthread_cond_t const cond = mtctx->jobs[jobID].job_cond;
        DEBUGLOG(4, "job%02u: release dst address %08X", jobID, (U32)(size_t)mtctx->jobs[jobID].dstBuff.start);
        ZSTDMT_releaseBuffer(mtctx->bufPool, mtctx->jobs[jobID].dstBuff);
        ZSTD_memset(&mtctx->jobs[jobID], 0, sizeof(mtctx->jobs[jobID]));
        mtctx->jobs[jobID].job_mutex = mutex;
        mtctx->jobs[jobID].job_cond = cond;
    }
    mtctx->inBuff.buffer = g_nullBuffer;
    mtctx->inBuff.filled = 0;
    mtctx->allJobsCompleted = 1;
}
static void ZSTDMT_waitForAllJobsCompleted(ZSTDMT_CCtx* mtctx)
{
    DEBUGLOG(4, "ZSTDMT_waitForAllJobsCompleted");
    while (mtctx->doneJobID < mtctx->nextJobID) {
        unsigned const jobID = mtctx->doneJobID & mtctx->jobIDMask;
        ZSTD_PTHREAD_MUTEX_LOCK(&mtctx->jobs[jobID].job_mutex);
        while (mtctx->jobs[jobID].consumed < mtctx->jobs[jobID].src.size) {
            DEBUGLOG(4, "waiting for jobCompleted signal from job %u", mtctx->doneJobID);
            ZSTD_pthread_cond_wait(&mtctx->jobs[jobID].job_cond, &mtctx->jobs[jobID].job_mutex);
        }
        ZSTD_pthread_mutex_unlock(&mtctx->jobs[jobID].job_mutex);
        mtctx->doneJobID++;
    }
}
size_t ZSTDMT_freeCCtx(ZSTDMT_CCtx* mtctx)
{
    if (mtctx==NULL) return 0;
    if (!mtctx->providedFactory)
        POOL_free(mtctx->factory);
    ZSTDMT_releaseAllJobResources(mtctx);
    ZSTDMT_freeJobsTable(mtctx->jobs, mtctx->jobIDMask+1, mtctx->cMem);
    ZSTDMT_freeBufferPool(mtctx->bufPool);
    ZSTDMT_freeCCtxPool(mtctx->cctxPool);
    ZSTDMT_freeSeqPool(mtctx->seqPool);
    ZSTDMT_serialState_free(&mtctx->serial);
    ZSTD_freeCDict(mtctx->cdictLocal);
    if (mtctx->roundBuff.buffer)
        ZSTD_customFree(mtctx->roundBuff.buffer, mtctx->cMem);
    ZSTD_customFree(mtctx, mtctx->cMem);
    return 0;
}
size_t ZSTDMT_sizeof_CCtx(ZSTDMT_CCtx* mtctx)
{
    if (mtctx == NULL) return 0;
    return sizeof(*mtctx)
            + POOL_sizeof(mtctx->factory)
            + ZSTDMT_sizeof_bufferPool(mtctx->bufPool)
            + (mtctx->jobIDMask+1) * sizeof(ZSTDMT_jobDescription)
            + ZSTDMT_sizeof_CCtxPool(mtctx->cctxPool)
            + ZSTDMT_sizeof_seqPool(mtctx->seqPool)
            + ZSTD_sizeof_CDict(mtctx->cdictLocal)
            + mtctx->roundBuff.capacity;
}
static size_t ZSTDMT_resize(ZSTDMT_CCtx* mtctx, unsigned nbWorkers)
{
    if (POOL_resize(mtctx->factory, nbWorkers)) return ERROR(memory_allocation);
    FORWARD_IF_ERROR( ZSTDMT_expandJobsTable(mtctx, nbWorkers) , "");
    mtctx->bufPool = ZSTDMT_expandBufferPool(mtctx->bufPool, BUF_POOL_MAX_NB_BUFFERS(nbWorkers));
    if (mtctx->bufPool == NULL) return ERROR(memory_allocation);
    mtctx->cctxPool = ZSTDMT_expandCCtxPool(mtctx->cctxPool, nbWorkers);
    if (mtctx->cctxPool == NULL) return ERROR(memory_allocation);
    mtctx->seqPool = ZSTDMT_expandSeqPool(mtctx->seqPool, nbWorkers);
    if (mtctx->seqPool == NULL) return ERROR(memory_allocation);
    ZSTDMT_CCtxParam_setNbWorkers(&mtctx->params, nbWorkers);
    return 0;
}
void ZSTDMT_updateCParams_whileCompressing(ZSTDMT_CCtx* mtctx, const ZSTD_CCtx_params* cctxParams)
{
    U32 const saved_wlog = mtctx->params.cParams.windowLog;
    int const compressionLevel = cctxParams->compressionLevel;
    DEBUGLOG(5, "ZSTDMT_updateCParams_whileCompressing (level:%i)",
                compressionLevel);
    mtctx->params.compressionLevel = compressionLevel;
    {   ZSTD_compressionParameters cParams = ZSTD_getCParamsFromCCtxParams(cctxParams, ZSTD_CONTENTSIZE_UNKNOWN, 0, ZSTD_cpm_noAttachDict);
        cParams.windowLog = saved_wlog;
        mtctx->params.cParams = cParams;
    }
}
ZSTD_frameProgression ZSTDMT_getFrameProgression(ZSTDMT_CCtx* mtctx)
{
    ZSTD_frameProgression fps;
    DEBUGLOG(5, "ZSTDMT_getFrameProgression");
    fps.ingested = mtctx->consumed + mtctx->inBuff.filled;
    fps.consumed = mtctx->consumed;
    fps.produced = fps.flushed = mtctx->produced;
    fps.currentJobID = mtctx->nextJobID;
    fps.nbActiveWorkers = 0;
    {   unsigned jobNb;
        unsigned lastJobNb = mtctx->nextJobID + mtctx->jobReady; assert(mtctx->jobReady <= 1);
        DEBUGLOG(6, "ZSTDMT_getFrameProgression: jobs: from %u to <%u (jobReady:%u)",
                    mtctx->doneJobID, lastJobNb, mtctx->jobReady);
        for (jobNb = mtctx->doneJobID ; jobNb < lastJobNb ; jobNb++) {
            unsigned const wJobID = jobNb & mtctx->jobIDMask;
            ZSTDMT_jobDescription* jobPtr = &mtctx->jobs[wJobID];
            ZSTD_pthread_mutex_lock(&jobPtr->job_mutex);
            {   size_t const cResult = jobPtr->cSize;
                size_t const produced = ZSTD_isError(cResult) ? 0 : cResult;
                size_t const flushed = ZSTD_isError(cResult) ? 0 : jobPtr->dstFlushed;
                assert(flushed <= produced);
                fps.ingested += jobPtr->src.size;
                fps.consumed += jobPtr->consumed;
                fps.produced += produced;
                fps.flushed  += flushed;
                fps.nbActiveWorkers += (jobPtr->consumed < jobPtr->src.size);
            }
            ZSTD_pthread_mutex_unlock(&mtctx->jobs[wJobID].job_mutex);
        }
    }
    return fps;
}
size_t ZSTDMT_toFlushNow(ZSTDMT_CCtx* mtctx)
{
    size_t toFlush;
    unsigned const jobID = mtctx->doneJobID;
    assert(jobID <= mtctx->nextJobID);
    if (jobID == mtctx->nextJobID) return 0;
    {   unsigned const wJobID = jobID & mtctx->jobIDMask;
        ZSTDMT_jobDescription* const jobPtr = &mtctx->jobs[wJobID];
        ZSTD_pthread_mutex_lock(&jobPtr->job_mutex);
        {   size_t const cResult = jobPtr->cSize;
            size_t const produced = ZSTD_isError(cResult) ? 0 : cResult;
            size_t const flushed = ZSTD_isError(cResult) ? 0 : jobPtr->dstFlushed;
            assert(flushed <= produced);
            assert(jobPtr->consumed <= jobPtr->src.size);
            toFlush = produced - flushed;
            if (toFlush==0) {
                assert(jobPtr->consumed < jobPtr->src.size);
            }
        }
        ZSTD_pthread_mutex_unlock(&mtctx->jobs[wJobID].job_mutex);
    }
    return toFlush;
}
static unsigned ZSTDMT_computeTargetJobLog(const ZSTD_CCtx_params* params)
{
    unsigned jobLog;
    if (params->ldmParams.enableLdm == ZSTD_ps_enable) {
        jobLog = MAX(21, ZSTD_cycleLog(params->cParams.chainLog, params->cParams.strategy) + 3);
    } else {
        jobLog = MAX(20, params->cParams.windowLog + 2);
    }
    return MIN(jobLog, (unsigned)ZSTDMT_JOBLOG_MAX);
}
static int ZSTDMT_overlapLog_default(ZSTD_strategy strat)
{
    switch(strat)
    {
        case ZSTD_btultra2:
            return 9;
        case ZSTD_btultra:
        case ZSTD_btopt:
            return 8;
        case ZSTD_btlazy2:
        case ZSTD_lazy2:
            return 7;
        case ZSTD_lazy:
        case ZSTD_greedy:
        case ZSTD_dfast:
        case ZSTD_fast:
        default:;
    }
    return 6;
}
static int ZSTDMT_overlapLog(int ovlog, ZSTD_strategy strat)
{
    assert(0 <= ovlog && ovlog <= 9);
    if (ovlog == 0) return ZSTDMT_overlapLog_default(strat);
    return ovlog;
}
static size_t ZSTDMT_computeOverlapSize(const ZSTD_CCtx_params* params)
{
    int const overlapRLog = 9 - ZSTDMT_overlapLog(params->overlapLog, params->cParams.strategy);
    int ovLog = (overlapRLog >= 8) ? 0 : (params->cParams.windowLog - overlapRLog);
    assert(0 <= overlapRLog && overlapRLog <= 8);
    if (params->ldmParams.enableLdm == ZSTD_ps_enable) {
        ovLog = MIN(params->cParams.windowLog, ZSTDMT_computeTargetJobLog(params) - 2)
                - overlapRLog;
    }
    assert(0 <= ovLog && ovLog <= ZSTD_WINDOWLOG_MAX);
    DEBUGLOG(4, "overlapLog : %i", params->overlapLog);
    DEBUGLOG(4, "overlap size : %i", 1 << ovLog);
    return (ovLog==0) ? 0 : (size_t)1 << ovLog;
}
size_t ZSTDMT_initCStream_internal(
        ZSTDMT_CCtx* mtctx,
        const void* dict, size_t dictSize, ZSTD_dictContentType_e dictContentType,
        const ZSTD_CDict* cdict, ZSTD_CCtx_params params,
        unsigned long long pledgedSrcSize)
{
    DEBUGLOG(4, "ZSTDMT_initCStream_internal (pledgedSrcSize=%u, nbWorkers=%u, cctxPool=%u)",
                (U32)pledgedSrcSize, params.nbWorkers, mtctx->cctxPool->totalCCtx);
    assert(!ZSTD_isError(ZSTD_checkCParams(params.cParams)));
    assert(!((dict) && (cdict)));
    if (params.nbWorkers != mtctx->params.nbWorkers)
        FORWARD_IF_ERROR( ZSTDMT_resize(mtctx, (unsigned)params.nbWorkers) , "");
    if (params.jobSize != 0 && params.jobSize < ZSTDMT_JOBSIZE_MIN) params.jobSize = ZSTDMT_JOBSIZE_MIN;
    if (params.jobSize > (size_t)ZSTDMT_JOBSIZE_MAX) params.jobSize = (size_t)ZSTDMT_JOBSIZE_MAX;
    if (mtctx->allJobsCompleted == 0) {
        ZSTDMT_waitForAllJobsCompleted(mtctx);
        ZSTDMT_releaseAllJobResources(mtctx);
        mtctx->allJobsCompleted = 1;
    }
    mtctx->params = params;
    mtctx->frameContentSize = pledgedSrcSize;
    ZSTD_freeCDict(mtctx->cdictLocal);
    if (dict) {
        mtctx->cdictLocal = ZSTD_createCDict_advanced(dict, dictSize,
                                                    ZSTD_dlm_byCopy, dictContentType,
                                                    params.cParams, mtctx->cMem);
        mtctx->cdict = mtctx->cdictLocal;
        if (mtctx->cdictLocal == NULL) return ERROR(memory_allocation);
    } else {
        mtctx->cdictLocal = NULL;
        mtctx->cdict = cdict;
    }
    mtctx->targetPrefixSize = ZSTDMT_computeOverlapSize(&params);
    DEBUGLOG(4, "overlapLog=%i => %u KB", params.overlapLog, (U32)(mtctx->targetPrefixSize>>10));
    mtctx->targetSectionSize = params.jobSize;
    if (mtctx->targetSectionSize == 0) {
        mtctx->targetSectionSize = 1ULL << ZSTDMT_computeTargetJobLog(&params);
    }
    assert(mtctx->targetSectionSize <= (size_t)ZSTDMT_JOBSIZE_MAX);
    if (params.rsyncable) {
        U32 const jobSizeKB = (U32)(mtctx->targetSectionSize >> 10);
        U32 const rsyncBits = (assert(jobSizeKB >= 1), ZSTD_highbit32(jobSizeKB) + 10);
        assert(rsyncBits >= RSYNC_MIN_BLOCK_LOG + 2);
        DEBUGLOG(4, "rsyncLog = %u", rsyncBits);
        mtctx->rsync.hash = 0;
        mtctx->rsync.hitMask = (1ULL << rsyncBits) - 1;
        mtctx->rsync.primePower = ZSTD_rollingHash_primePower(RSYNC_LENGTH);
    }
    if (mtctx->targetSectionSize < mtctx->targetPrefixSize) mtctx->targetSectionSize = mtctx->targetPrefixSize;
    DEBUGLOG(4, "Job Size : %u KB (note : set to %u)", (U32)(mtctx->targetSectionSize>>10), (U32)params.jobSize);
    DEBUGLOG(4, "inBuff Size : %u KB", (U32)(mtctx->targetSectionSize>>10));
    ZSTDMT_setBufferSize(mtctx->bufPool, ZSTD_compressBound(mtctx->targetSectionSize));
    {
        size_t const windowSize = mtctx->params.ldmParams.enableLdm == ZSTD_ps_enable ? (1U << mtctx->params.cParams.windowLog) : 0;
        size_t const nbSlackBuffers = 2 + (mtctx->targetPrefixSize > 0);
        size_t const slackSize = mtctx->targetSectionSize * nbSlackBuffers;
        size_t const nbWorkers = MAX(mtctx->params.nbWorkers, 1);
        size_t const sectionsSize = mtctx->targetSectionSize * nbWorkers;
        size_t const capacity = MAX(windowSize, sectionsSize) + slackSize;
        if (mtctx->roundBuff.capacity < capacity) {
            if (mtctx->roundBuff.buffer)
                ZSTD_customFree(mtctx->roundBuff.buffer, mtctx->cMem);
            mtctx->roundBuff.buffer = (BYTE*)ZSTD_customMalloc(capacity, mtctx->cMem);
            if (mtctx->roundBuff.buffer == NULL) {
                mtctx->roundBuff.capacity = 0;
                return ERROR(memory_allocation);
            }
            mtctx->roundBuff.capacity = capacity;
        }
    }
    DEBUGLOG(4, "roundBuff capacity : %u KB", (U32)(mtctx->roundBuff.capacity>>10));
    mtctx->roundBuff.pos = 0;
    mtctx->inBuff.buffer = g_nullBuffer;
    mtctx->inBuff.filled = 0;
    mtctx->inBuff.prefix = kNullRange;
    mtctx->doneJobID = 0;
    mtctx->nextJobID = 0;
    mtctx->frameEnded = 0;
    mtctx->allJobsCompleted = 0;
    mtctx->consumed = 0;
    mtctx->produced = 0;
    ZSTD_freeCDict(mtctx->cdictLocal);
    mtctx->cdictLocal = NULL;
    mtctx->cdict = NULL;
    if (dict) {
        if (dictContentType == ZSTD_dct_rawContent) {
            mtctx->inBuff.prefix.start = (const BYTE*)dict;
            mtctx->inBuff.prefix.size = dictSize;
        } else {
            mtctx->cdictLocal = ZSTD_createCDict_advanced(dict, dictSize,
                                                        ZSTD_dlm_byRef, dictContentType,
                                                        params.cParams, mtctx->cMem);
            mtctx->cdict = mtctx->cdictLocal;
            if (mtctx->cdictLocal == NULL) return ERROR(memory_allocation);
        }
    } else {
        mtctx->cdict = cdict;
    }
    if (ZSTDMT_serialState_reset(&mtctx->serial, mtctx->seqPool, params, mtctx->targetSectionSize,
                                 dict, dictSize, dictContentType))
        return ERROR(memory_allocation);
    return 0;
}
static void ZSTDMT_writeLastEmptyBlock(ZSTDMT_jobDescription* job)
{
    assert(job->lastJob == 1);
    assert(job->src.size == 0);
    assert(job->firstJob == 0);
    assert(job->dstBuff.start == NULL);
    job->dstBuff = ZSTDMT_getBuffer(job->bufPool);
    if (job->dstBuff.start == NULL) {
      job->cSize = ERROR(memory_allocation);
      return;
    }
    assert(job->dstBuff.capacity >= ZSTD_blockHeaderSize);
    job->src = kNullRange;
    job->cSize = ZSTD_writeLastEmptyBlock(job->dstBuff.start, job->dstBuff.capacity);
    assert(!ZSTD_isError(job->cSize));
    assert(job->consumed == 0);
}
static size_t ZSTDMT_createCompressionJob(ZSTDMT_CCtx* mtctx, size_t srcSize, ZSTD_EndDirective endOp)
{
    unsigned const jobID = mtctx->nextJobID & mtctx->jobIDMask;
    int const endFrame = (endOp == ZSTD_e_end);
    if (mtctx->nextJobID > mtctx->doneJobID + mtctx->jobIDMask) {
        DEBUGLOG(5, "ZSTDMT_createCompressionJob: will not create new job : table is full");
        assert((mtctx->nextJobID & mtctx->jobIDMask) == (mtctx->doneJobID & mtctx->jobIDMask));
        return 0;
    }
    if (!mtctx->jobReady) {
        BYTE const* src = (BYTE const*)mtctx->inBuff.buffer.start;
        DEBUGLOG(5, "ZSTDMT_createCompressionJob: preparing job %u to compress %u bytes with %u preload ",
                    mtctx->nextJobID, (U32)srcSize, (U32)mtctx->inBuff.prefix.size);
        mtctx->jobs[jobID].src.start = src;
        mtctx->jobs[jobID].src.size = srcSize;
        assert(mtctx->inBuff.filled >= srcSize);
        mtctx->jobs[jobID].prefix = mtctx->inBuff.prefix;
        mtctx->jobs[jobID].consumed = 0;
        mtctx->jobs[jobID].cSize = 0;
        mtctx->jobs[jobID].params = mtctx->params;
        mtctx->jobs[jobID].cdict = mtctx->nextJobID==0 ? mtctx->cdict : NULL;
        mtctx->jobs[jobID].fullFrameSize = mtctx->frameContentSize;
        mtctx->jobs[jobID].dstBuff = g_nullBuffer;
        mtctx->jobs[jobID].cctxPool = mtctx->cctxPool;
        mtctx->jobs[jobID].bufPool = mtctx->bufPool;
        mtctx->jobs[jobID].seqPool = mtctx->seqPool;
        mtctx->jobs[jobID].serial = &mtctx->serial;
        mtctx->jobs[jobID].jobID = mtctx->nextJobID;
        mtctx->jobs[jobID].firstJob = (mtctx->nextJobID==0);
        mtctx->jobs[jobID].lastJob = endFrame;
        mtctx->jobs[jobID].frameChecksumNeeded = mtctx->params.fParams.checksumFlag && endFrame && (mtctx->nextJobID>0);
        mtctx->jobs[jobID].dstFlushed = 0;
        mtctx->roundBuff.pos += srcSize;
        mtctx->inBuff.buffer = g_nullBuffer;
        mtctx->inBuff.filled = 0;
        if (!endFrame) {
            size_t const newPrefixSize = MIN(srcSize, mtctx->targetPrefixSize);
            mtctx->inBuff.prefix.start = src + srcSize - newPrefixSize;
            mtctx->inBuff.prefix.size = newPrefixSize;
        } else {
            mtctx->inBuff.prefix = kNullRange;
            mtctx->frameEnded = endFrame;
            if (mtctx->nextJobID == 0) {
                mtctx->params.fParams.checksumFlag = 0;
        }   }
        if ( (srcSize == 0)
          && (mtctx->nextJobID>0) ) {
            DEBUGLOG(5, "ZSTDMT_createCompressionJob: creating a last empty block to end frame");
            assert(endOp == ZSTD_e_end);
            ZSTDMT_writeLastEmptyBlock(mtctx->jobs + jobID);
            mtctx->nextJobID++;
            return 0;
        }
    }
    DEBUGLOG(5, "ZSTDMT_createCompressionJob: posting job %u : %u bytes  (end:%u, jobNb == %u (mod:%u))",
                mtctx->nextJobID,
                (U32)mtctx->jobs[jobID].src.size,
                mtctx->jobs[jobID].lastJob,
                mtctx->nextJobID,
                jobID);
    if (POOL_tryAdd(mtctx->factory, ZSTDMT_compressionJob, &mtctx->jobs[jobID])) {
        mtctx->nextJobID++;
        mtctx->jobReady = 0;
    } else {
        DEBUGLOG(5, "ZSTDMT_createCompressionJob: no worker available for job %u", mtctx->nextJobID);
        mtctx->jobReady = 1;
    }
    return 0;
}
static size_t ZSTDMT_flushProduced(ZSTDMT_CCtx* mtctx, ZSTD_outBuffer* output, unsigned blockToFlush, ZSTD_EndDirective end)
{
    unsigned const wJobID = mtctx->doneJobID & mtctx->jobIDMask;
    DEBUGLOG(5, "ZSTDMT_flushProduced (blocking:%u , job %u <= %u)",
                blockToFlush, mtctx->doneJobID, mtctx->nextJobID);
    assert(output->size >= output->pos);
    ZSTD_PTHREAD_MUTEX_LOCK(&mtctx->jobs[wJobID].job_mutex);
    if (  blockToFlush
      && (mtctx->doneJobID < mtctx->nextJobID) ) {
        assert(mtctx->jobs[wJobID].dstFlushed <= mtctx->jobs[wJobID].cSize);
        while (mtctx->jobs[wJobID].dstFlushed == mtctx->jobs[wJobID].cSize) {
            if (mtctx->jobs[wJobID].consumed == mtctx->jobs[wJobID].src.size) {
                DEBUGLOG(5, "job %u is completely consumed (%u == %u) => don't wait for cond, there will be none",
                            mtctx->doneJobID, (U32)mtctx->jobs[wJobID].consumed, (U32)mtctx->jobs[wJobID].src.size);
                break;
            }
            DEBUGLOG(5, "waiting for something to flush from job %u (currently flushed: %u bytes)",
                        mtctx->doneJobID, (U32)mtctx->jobs[wJobID].dstFlushed);
            ZSTD_pthread_cond_wait(&mtctx->jobs[wJobID].job_cond, &mtctx->jobs[wJobID].job_mutex);
    }   }
    {   size_t cSize = mtctx->jobs[wJobID].cSize;
        size_t const srcConsumed = mtctx->jobs[wJobID].consumed;
        size_t const srcSize = mtctx->jobs[wJobID].src.size;
        ZSTD_pthread_mutex_unlock(&mtctx->jobs[wJobID].job_mutex);
        if (ZSTD_isError(cSize)) {
            DEBUGLOG(5, "ZSTDMT_flushProduced: job %u : compression error detected : %s",
                        mtctx->doneJobID, ZSTD_getErrorName(cSize));
            ZSTDMT_waitForAllJobsCompleted(mtctx);
            ZSTDMT_releaseAllJobResources(mtctx);
            return cSize;
        }
        assert(srcConsumed <= srcSize);
        if ( (srcConsumed == srcSize)
          && mtctx->jobs[wJobID].frameChecksumNeeded ) {
            U32 const checksum = (U32)XXH64_digest(&mtctx->serial.xxhState);
            DEBUGLOG(4, "ZSTDMT_flushProduced: writing checksum : %08X \n", checksum);
            MEM_writeLE32((char*)mtctx->jobs[wJobID].dstBuff.start + mtctx->jobs[wJobID].cSize, checksum);
            cSize += 4;
            mtctx->jobs[wJobID].cSize += 4;
            mtctx->jobs[wJobID].frameChecksumNeeded = 0;
        }
        if (cSize > 0) {
            size_t const toFlush = MIN(cSize - mtctx->jobs[wJobID].dstFlushed, output->size - output->pos);
            DEBUGLOG(5, "ZSTDMT_flushProduced: Flushing %u bytes from job %u (completion:%u/%u, generated:%u)",
                        (U32)toFlush, mtctx->doneJobID, (U32)srcConsumed, (U32)srcSize, (U32)cSize);
            assert(mtctx->doneJobID < mtctx->nextJobID);
            assert(cSize >= mtctx->jobs[wJobID].dstFlushed);
            assert(mtctx->jobs[wJobID].dstBuff.start != NULL);
            if (toFlush > 0) {
                ZSTD_memcpy((char*)output->dst + output->pos,
                    (const char*)mtctx->jobs[wJobID].dstBuff.start + mtctx->jobs[wJobID].dstFlushed,
                    toFlush);
            }
            output->pos += toFlush;
            mtctx->jobs[wJobID].dstFlushed += toFlush;
            if ( (srcConsumed == srcSize)
              && (mtctx->jobs[wJobID].dstFlushed == cSize) ) {
                DEBUGLOG(5, "Job %u completed (%u bytes), moving to next one",
                        mtctx->doneJobID, (U32)mtctx->jobs[wJobID].dstFlushed);
                ZSTDMT_releaseBuffer(mtctx->bufPool, mtctx->jobs[wJobID].dstBuff);
                DEBUGLOG(5, "dstBuffer released");
                mtctx->jobs[wJobID].dstBuff = g_nullBuffer;
                mtctx->jobs[wJobID].cSize = 0;
                mtctx->consumed += srcSize;
                mtctx->produced += cSize;
                mtctx->doneJobID++;
        }   }
        if (cSize > mtctx->jobs[wJobID].dstFlushed) return (cSize - mtctx->jobs[wJobID].dstFlushed);
        if (srcSize > srcConsumed) return 1;
    }
    if (mtctx->doneJobID < mtctx->nextJobID) return 1;
    if (mtctx->jobReady) return 1;
    if (mtctx->inBuff.filled > 0) return 1;
    mtctx->allJobsCompleted = mtctx->frameEnded;
    if (end == ZSTD_e_end) return !mtctx->frameEnded;
    return 0;
}
static Range ZSTDMT_getInputDataInUse(ZSTDMT_CCtx* mtctx)
{
    unsigned const firstJobID = mtctx->doneJobID;
    unsigned const lastJobID = mtctx->nextJobID;
    unsigned jobID;
    size_t roundBuffCapacity = mtctx->roundBuff.capacity;
    size_t nbJobs1stRoundMin = roundBuffCapacity / mtctx->targetSectionSize;
    if (lastJobID < nbJobs1stRoundMin) return kNullRange;
    for (jobID = firstJobID; jobID < lastJobID; ++jobID) {
        unsigned const wJobID = jobID & mtctx->jobIDMask;
        size_t consumed;
        ZSTD_PTHREAD_MUTEX_LOCK(&mtctx->jobs[wJobID].job_mutex);
        consumed = mtctx->jobs[wJobID].consumed;
        ZSTD_pthread_mutex_unlock(&mtctx->jobs[wJobID].job_mutex);
        if (consumed < mtctx->jobs[wJobID].src.size) {
            Range range = mtctx->jobs[wJobID].prefix;
            if (range.size == 0) {
                range = mtctx->jobs[wJobID].src;
            }
            assert(range.start <= mtctx->jobs[wJobID].src.start);
            return range;
        }
    }
    return kNullRange;
}
static int ZSTDMT_isOverlapped(Buffer buffer, Range range)
{
    BYTE const* const bufferStart = (BYTE const*)buffer.start;
    BYTE const* const rangeStart = (BYTE const*)range.start;
    if (rangeStart == NULL || bufferStart == NULL)
        return 0;
    {
        BYTE const* const bufferEnd = bufferStart + buffer.capacity;
        BYTE const* const rangeEnd = rangeStart + range.size;
        if (bufferStart == bufferEnd || rangeStart == rangeEnd)
            return 0;
        return bufferStart < rangeEnd && rangeStart < bufferEnd;
    }
}
static int ZSTDMT_doesOverlapWindow(Buffer buffer, ZSTD_window_t window)
{
    Range extDict;
    Range prefix;
    DEBUGLOG(5, "ZSTDMT_doesOverlapWindow");
    extDict.start = window.dictBase + window.lowLimit;
    extDict.size = window.dictLimit - window.lowLimit;
    prefix.start = window.base + window.dictLimit;
    prefix.size = window.nextSrc - (window.base + window.dictLimit);
    DEBUGLOG(5, "extDict [0x%zx, 0x%zx)",
                (size_t)extDict.start,
                (size_t)extDict.start + extDict.size);
    DEBUGLOG(5, "prefix  [0x%zx, 0x%zx)",
                (size_t)prefix.start,
                (size_t)prefix.start + prefix.size);
    return ZSTDMT_isOverlapped(buffer, extDict)
        || ZSTDMT_isOverlapped(buffer, prefix);
}
static void ZSTDMT_waitForLdmComplete(ZSTDMT_CCtx* mtctx, Buffer buffer)
{
    if (mtctx->params.ldmParams.enableLdm == ZSTD_ps_enable) {
        ZSTD_pthread_mutex_t* mutex = &mtctx->serial.ldmWindowMutex;
        DEBUGLOG(5, "ZSTDMT_waitForLdmComplete");
        DEBUGLOG(5, "source  [0x%zx, 0x%zx)",
                    (size_t)buffer.start,
                    (size_t)buffer.start + buffer.capacity);
        ZSTD_PTHREAD_MUTEX_LOCK(mutex);
        while (ZSTDMT_doesOverlapWindow(buffer, mtctx->serial.ldmWindow)) {
            DEBUGLOG(5, "Waiting for LDM to finish...");
            ZSTD_pthread_cond_wait(&mtctx->serial.ldmWindowCond, mutex);
        }
        DEBUGLOG(6, "Done waiting for LDM to finish");
        ZSTD_pthread_mutex_unlock(mutex);
    }
}
static int ZSTDMT_tryGetInputRange(ZSTDMT_CCtx* mtctx)
{
    Range const inUse = ZSTDMT_getInputDataInUse(mtctx);
    size_t const spaceLeft = mtctx->roundBuff.capacity - mtctx->roundBuff.pos;
    size_t const spaceNeeded = mtctx->targetSectionSize;
    Buffer buffer;
    DEBUGLOG(5, "ZSTDMT_tryGetInputRange");
    assert(mtctx->inBuff.buffer.start == NULL);
    assert(mtctx->roundBuff.capacity >= spaceNeeded);
    if (spaceLeft < spaceNeeded) {
        BYTE* const start = (BYTE*)mtctx->roundBuff.buffer;
        size_t const prefixSize = mtctx->inBuff.prefix.size;
        buffer.start = start;
        buffer.capacity = prefixSize;
        if (ZSTDMT_isOverlapped(buffer, inUse)) {
            DEBUGLOG(5, "Waiting for buffer...");
            return 0;
        }
        ZSTDMT_waitForLdmComplete(mtctx, buffer);
        ZSTD_memmove(start, mtctx->inBuff.prefix.start, prefixSize);
        mtctx->inBuff.prefix.start = start;
        mtctx->roundBuff.pos = prefixSize;
    }
    buffer.start = mtctx->roundBuff.buffer + mtctx->roundBuff.pos;
    buffer.capacity = spaceNeeded;
    if (ZSTDMT_isOverlapped(buffer, inUse)) {
        DEBUGLOG(5, "Waiting for buffer...");
        return 0;
    }
    assert(!ZSTDMT_isOverlapped(buffer, mtctx->inBuff.prefix));
    ZSTDMT_waitForLdmComplete(mtctx, buffer);
    DEBUGLOG(5, "Using prefix range [%zx, %zx)",
                (size_t)mtctx->inBuff.prefix.start,
                (size_t)mtctx->inBuff.prefix.start + mtctx->inBuff.prefix.size);
    DEBUGLOG(5, "Using source range [%zx, %zx)",
                (size_t)buffer.start,
                (size_t)buffer.start + buffer.capacity);
    mtctx->inBuff.buffer = buffer;
    mtctx->inBuff.filled = 0;
    assert(mtctx->roundBuff.pos + buffer.capacity <= mtctx->roundBuff.capacity);
    return 1;
}
typedef struct {
  size_t toLoad;
  int flush;
} SyncPoint;
static SyncPoint
findSynchronizationPoint(ZSTDMT_CCtx const* mtctx, ZSTD_inBuffer const input)
{
    BYTE const* const istart = (BYTE const*)input.src + input.pos;
    U64 const primePower = mtctx->rsync.primePower;
    U64 const hitMask = mtctx->rsync.hitMask;
    SyncPoint syncPoint;
    U64 hash;
    BYTE const* prev;
    size_t pos;
    syncPoint.toLoad = MIN(input.size - input.pos, mtctx->targetSectionSize - mtctx->inBuff.filled);
    syncPoint.flush = 0;
    if (!mtctx->params.rsyncable)
        return syncPoint;
    if (mtctx->inBuff.filled + input.size - input.pos < RSYNC_MIN_BLOCK_SIZE)
        return syncPoint;
    if (mtctx->inBuff.filled + syncPoint.toLoad < RSYNC_LENGTH)
        return syncPoint;
    if (mtctx->inBuff.filled < RSYNC_MIN_BLOCK_SIZE) {
        pos = RSYNC_MIN_BLOCK_SIZE - mtctx->inBuff.filled;
        if (pos >= RSYNC_LENGTH) {
            prev = istart + pos - RSYNC_LENGTH;
            hash = ZSTD_rollingHash_compute(prev, RSYNC_LENGTH);
        } else {
            assert(mtctx->inBuff.filled >= RSYNC_LENGTH);
            prev = (BYTE const*)mtctx->inBuff.buffer.start + mtctx->inBuff.filled - RSYNC_LENGTH;
            hash = ZSTD_rollingHash_compute(prev + pos, (RSYNC_LENGTH - pos));
            hash = ZSTD_rollingHash_append(hash, istart, pos);
        }
    } else {
        assert(mtctx->inBuff.filled >= RSYNC_MIN_BLOCK_SIZE);
        assert(RSYNC_MIN_BLOCK_SIZE >= RSYNC_LENGTH);
        pos = 0;
        prev = (BYTE const*)mtctx->inBuff.buffer.start + mtctx->inBuff.filled - RSYNC_LENGTH;
        hash = ZSTD_rollingHash_compute(prev, RSYNC_LENGTH);
        if ((hash & hitMask) == hitMask) {
            syncPoint.toLoad = 0;
            syncPoint.flush = 1;
            return syncPoint;
        }
    }
    assert(pos < RSYNC_LENGTH || ZSTD_rollingHash_compute(istart + pos - RSYNC_LENGTH, RSYNC_LENGTH) == hash);
    for (; pos < syncPoint.toLoad; ++pos) {
        BYTE const toRemove = pos < RSYNC_LENGTH ? prev[pos] : istart[pos - RSYNC_LENGTH];
        hash = ZSTD_rollingHash_rotate(hash, toRemove, istart[pos], primePower);
        assert(mtctx->inBuff.filled + pos >= RSYNC_MIN_BLOCK_SIZE);
        if ((hash & hitMask) == hitMask) {
            syncPoint.toLoad = pos + 1;
            syncPoint.flush = 1;
            ++pos;
            break;
        }
    }
    assert(pos < RSYNC_LENGTH || ZSTD_rollingHash_compute(istart + pos - RSYNC_LENGTH, RSYNC_LENGTH) == hash);
    return syncPoint;
}
size_t ZSTDMT_nextInputSizeHint(const ZSTDMT_CCtx* mtctx)
{
    size_t hintInSize = mtctx->targetSectionSize - mtctx->inBuff.filled;
    if (hintInSize==0) hintInSize = mtctx->targetSectionSize;
    return hintInSize;
}
size_t ZSTDMT_compressStream_generic(ZSTDMT_CCtx* mtctx,
                                     ZSTD_outBuffer* output,
                                     ZSTD_inBuffer* input,
                                     ZSTD_EndDirective endOp)
{
    unsigned forwardInputProgress = 0;
    DEBUGLOG(5, "ZSTDMT_compressStream_generic (endOp=%u, srcSize=%u)",
                (U32)endOp, (U32)(input->size - input->pos));
    assert(output->pos <= output->size);
    assert(input->pos  <= input->size);
    if ((mtctx->frameEnded) && (endOp==ZSTD_e_continue)) {
        return ERROR(stage_wrong);
    }
    if ( (!mtctx->jobReady)
      && (input->size > input->pos) ) {
        if (mtctx->inBuff.buffer.start == NULL) {
            assert(mtctx->inBuff.filled == 0);
            if (!ZSTDMT_tryGetInputRange(mtctx)) {
                DEBUGLOG(5, "ZSTDMT_tryGetInputRange failed");
                assert(mtctx->doneJobID != mtctx->nextJobID);
            } else
                DEBUGLOG(5, "ZSTDMT_tryGetInputRange completed successfully : mtctx->inBuff.buffer.start = %p", mtctx->inBuff.buffer.start);
        }
        if (mtctx->inBuff.buffer.start != NULL) {
            SyncPoint const syncPoint = findSynchronizationPoint(mtctx, *input);
            if (syncPoint.flush && endOp == ZSTD_e_continue) {
                endOp = ZSTD_e_flush;
            }
            assert(mtctx->inBuff.buffer.capacity >= mtctx->targetSectionSize);
            DEBUGLOG(5, "ZSTDMT_compressStream_generic: adding %u bytes on top of %u to buffer of size %u",
                        (U32)syncPoint.toLoad, (U32)mtctx->inBuff.filled, (U32)mtctx->targetSectionSize);
            ZSTD_memcpy((char*)mtctx->inBuff.buffer.start + mtctx->inBuff.filled, (const char*)input->src + input->pos, syncPoint.toLoad);
            input->pos += syncPoint.toLoad;
            mtctx->inBuff.filled += syncPoint.toLoad;
            forwardInputProgress = syncPoint.toLoad>0;
        }
    }
    if ((input->pos < input->size) && (endOp == ZSTD_e_end)) {
        assert(mtctx->inBuff.filled == 0 || mtctx->inBuff.filled == mtctx->targetSectionSize || mtctx->params.rsyncable);
        endOp = ZSTD_e_flush;
    }
    if ( (mtctx->jobReady)
      || (mtctx->inBuff.filled >= mtctx->targetSectionSize)
      || ((endOp != ZSTD_e_continue) && (mtctx->inBuff.filled > 0))
      || ((endOp == ZSTD_e_end) && (!mtctx->frameEnded)) ) {
        size_t const jobSize = mtctx->inBuff.filled;
        assert(mtctx->inBuff.filled <= mtctx->targetSectionSize);
        FORWARD_IF_ERROR( ZSTDMT_createCompressionJob(mtctx, jobSize, endOp) , "");
    }
    {   size_t const remainingToFlush = ZSTDMT_flushProduced(mtctx, output, !forwardInputProgress, endOp);
        if (input->pos < input->size) return MAX(remainingToFlush, 1);
        DEBUGLOG(5, "end of ZSTDMT_compressStream_generic: remainingToFlush = %u", (U32)remainingToFlush);
        return remainingToFlush;
    }
}