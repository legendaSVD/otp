#ifndef ERL_IO_QUEUE_H__TYPES__
#define ERL_IO_QUEUE_H__TYPES__
#define ERTS_BINARY_TYPES_ONLY__
#include "erl_binary.h"
#undef ERTS_BINARY_TYPES_ONLY__
#include "erl_nif.h"
#ifdef DEBUG
#define MAX_SYSIOVEC_IOVLEN (1ull << (32 - 1))
#else
#define MAX_SYSIOVEC_IOVLEN (1ull << (sizeof(((SysIOVec*)0)->iov_len) * 8 - 1))
#endif
#define ERTS_SMALL_IO_QUEUE 5
typedef union {
    ErlDrvBinary driver;
    Binary nif;
} ErtsIOQBinary;
typedef struct {
    int vsize;
    Uint size;
    SysIOVec* iov;
    ErtsIOQBinary** binv;
} ErtsIOVecCommon;
typedef union {
    ErtsIOVecCommon common;
    ErlIOVec driver;
    ErlNifIOVec nif;
} ErtsIOVec;
typedef struct erts_io_queue {
    ErtsAlcType_t alct;
    int driver;
    Uint size;
    SysIOVec* v_start;
    SysIOVec* v_end;
    SysIOVec* v_head;
    SysIOVec* v_tail;
    SysIOVec  v_small[ERTS_SMALL_IO_QUEUE];
    ErtsIOQBinary **b_start;
    ErtsIOQBinary **b_end;
    ErtsIOQBinary **b_head;
    ErtsIOQBinary **b_tail;
    ErtsIOQBinary  *b_small[ERTS_SMALL_IO_QUEUE];
} ErtsIOQueue;
#endif
#if !defined(ERL_IO_QUEUE_H) && !defined(ERTS_IO_QUEUE_TYPES_ONLY__)
#define ERL_IO_QUEUE_H
#include "erl_binary.h"
#include "erl_bits.h"
void erts_ioq_init(ErtsIOQueue *q, ErtsAlcType_t alct, int driver);
void erts_ioq_clear(ErtsIOQueue *q);
Uint erts_ioq_size(const ErtsIOQueue *q);
int erts_ioq_enqv(ErtsIOQueue *q, ErtsIOVec *vec, Uint skip);
int erts_ioq_pushqv(ErtsIOQueue *q, ErtsIOVec *vec, Uint skip);
int erts_ioq_deq(ErtsIOQueue *q, Uint Uint);
Uint erts_ioq_peekqv(const ErtsIOQueue *q, ErtsIOVec *ev);
SysIOVec *erts_ioq_peekq(const ErtsIOQueue *q, int *vlenp);
Uint erts_ioq_sizeq(const ErtsIOQueue *q);
int erts_ioq_iolist_vec_len(Eterm obj, int* vsize, Uint* csize,
                            Uint* pvsize, Uint* pcsize,
                            size_t* total_size, Uint blimit);
int erts_ioq_iolist_to_vec(Eterm obj, SysIOVec* iov,
                           ErtsIOQBinary** binv, ErtsIOQBinary* cbin,
                           Uint bin_limit, int driver_binary);
ERTS_GLB_INLINE
int erts_ioq_iodata_vec_len(Eterm obj, int* vsize, Uint* csize,
                            Uint* pvsize, Uint* pcsize,
                            size_t* total_size, Uint blimit);
ERTS_GLB_INLINE
int erts_ioq_iodata_to_vec(Eterm obj, SysIOVec* iov,
                           ErtsIOQBinary** binv, ErtsIOQBinary* cbin,
                           Uint bin_limit, int driver_binary);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE
int erts_ioq_iodata_vec_len(Eterm obj, int* vsize, Uint* csize,
                            Uint* pvsize, Uint* pcsize,
                            size_t* total_size, Uint blimit) {
    if (is_bitstring(obj)) {
        ERTS_DECLARE_DUMMY(Eterm br_flags);
        ERTS_DECLARE_DUMMY(byte *base);
        Uint offset, size;
        BinRef *br;
        ERTS_GET_BITSTRING_REF(obj, br_flags, br, base, offset, size);
        if (size < MAX_SYSIOVEC_IOVLEN &&
            BIT_OFFSET(offset) == 0 &&
            TAIL_BITS(size) == 0) {
            size = BYTE_SIZE(size);
            *vsize = 1;
            *pvsize = 1;
            if (br) {
                *csize = 0;
                *pcsize = 0;
            } else {
                *csize = size;
                *pcsize = size;
            }
            *total_size = size;
            return 0;
        }
    }
    return erts_ioq_iolist_vec_len(obj, vsize, csize,
                                   pvsize, pcsize, total_size, blimit);
}
ERTS_GLB_INLINE
int erts_ioq_iodata_to_vec(Eterm obj,
                           SysIOVec *iov,
                           ErtsIOQBinary **binv,
                           ErtsIOQBinary  *cbin,
                           Uint bin_limit,
                           int driver)
{
    if (is_bitstring(obj)) {
        ERTS_DECLARE_DUMMY(Eterm br_flags);
        Uint offset, size;
        byte *base;
        BinRef *br;
        ERTS_PIN_BITSTRING(obj, br_flags, br, base, offset, size);
        ASSERT(TAIL_BITS(size) == 0);
        if (NBYTES(size) < MAX_SYSIOVEC_IOVLEN) {
            if (br && BIT_OFFSET(offset) == 0) {
                Binary *refc_binary = br->val;
                iov[0].iov_base = &base[BYTE_OFFSET(offset)];
                iov[0].iov_len = NBYTES(size);
                if (driver) {
                    binv[0] = (ErtsIOQBinary*)Binary2ErlDrvBinary(refc_binary);
                } else {
                    binv[0] = (ErtsIOQBinary*)refc_binary;
                }
                return 1;
            } else if (br == NULL) {
                byte *buf = driver ? (byte*)cbin->driver.orig_bytes :
                                     (byte*)cbin->nif.orig_bytes;
                copy_binary_to_buffer(buf, 0, base, offset, size);
                iov[0].iov_base = buf;
                iov[0].iov_len = NBYTES(size);
                binv[0] = cbin;
                return 1;
            }
        }
    }
    return erts_ioq_iolist_to_vec(obj, iov, binv, cbin, bin_limit, driver);
}
#endif
#endif