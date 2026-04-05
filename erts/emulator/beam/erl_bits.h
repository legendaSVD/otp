#ifndef __ERL_BITS_H__
#define __ERL_BITS_H__
#define NBYTES(x)  (((Uint64)(x) + (Uint64) 7) >> 3)
#define NBITS(x)  ((Uint64)(x) << 3)
#define ERL_BITS_PER_REDUCTION (8*1024)
#define BYTE_OFFSET(offset_in_bits) ((Uint)(offset_in_bits) >> 3)
#define BIT_OFFSET(offset_in_bits) ((offset_in_bits) & 7)
#define BYTE_SIZE(size_in_bits) BYTE_OFFSET(size_in_bits)
#define TAIL_BITS(size_in_bits) BIT_OFFSET(size_in_bits)
#define bitstring_size(Bin)                                                   \
    ((bitstring_val(Bin)[0]) == HEADER_SUB_BITS ?                             \
     (((ErlSubBits*)bitstring_val(Bin))->end -                                \
      ((ErlSubBits*)bitstring_val(Bin))->start) :                             \
     (((ErlHeapBits*)bitstring_val(Bin))->size))
typedef struct erl_sub_bits {
    Eterm thing_word;
    UWord base_flags;
    Uint start;
    Uint end;
    Eterm orig;
} ErlSubBits;
#define ERL_SUB_BITS_FLAGS_MATCH_CONTEXT                                      \
    ERL_SUB_BITS_FLAG_MUTABLE
#define ERL_SUB_BITS_FLAGS_WRITABLE                                           \
    (ERL_SUB_BITS_FLAG_MUTABLE | ERL_SUB_BITS_FLAG_VOLATILE)
#define ERL_SUB_BITS_FLAG_MUTABLE ((UWord)(1 << 0))
#define ERL_SUB_BITS_FLAG_VOLATILE ((UWord)(1 << 1))
#define ERL_SUB_BITS_FLAG_MASK ((UWord)3)
ERTS_GLB_INLINE void
erl_sub_bits_init(ErlSubBits *sb, UWord flags, Eterm orig, const void *base,
                  Uint offset, Uint size);
ERTS_GLB_INLINE void
erl_sub_bits_update_moved(ErlSubBits *sb, Eterm orig);
#define erl_sub_bits_is_normal(SubBits)                                       \
    (erl_sub_bits_get_flags(SubBits) == 0)
#define erl_sub_bits_is_match_context(SubBits)                                \
    (erl_sub_bits_get_flags(SubBits) == ERL_SUB_BITS_FLAG_MUTABLE)
#define erl_sub_bits_is_writable(SubBits)                                     \
    (erl_sub_bits_get_flags(SubBits) ==                                       \
     (ERL_SUB_BITS_FLAG_MUTABLE | ERL_SUB_BITS_FLAG_VOLATILE))
#define erl_sub_bits_was_writable(SubBits)                                    \
    (erl_sub_bits_get_flags(SubBits) & ERL_SUB_BITS_FLAG_VOLATILE)
#define erl_sub_bits_clear_writable(SubBits)                                  \
    (ASSERT(erl_sub_bits_is_writable(SubBits)),                               \
     (SubBits)->base_flags &= ~ERL_SUB_BITS_FLAG_MUTABLE)
#define erl_sub_bits_get_flags(SubBits)                                       \
    ((SubBits)->base_flags & ERL_SUB_BITS_FLAG_MASK)
#define erl_sub_bits_get_base(SubBits)                                        \
    ((byte*)((SubBits)->base_flags & ~ERL_SUB_BITS_FLAG_MASK))
#define ERL_SUB_BITS_SIZE (sizeof(ErlSubBits) / sizeof(Eterm))
#define HEADER_SUB_BITS _make_header(ERL_SUB_BITS_SIZE-1,_TAG_HEADER_SUB_BITS)
typedef struct bin_ref {
    Eterm thing_word;
    Binary *val;
    struct erl_off_heap_header *next;
} BinRef;
#define HEADER_BIN_REF _make_header(ERL_BIN_REF_SIZE-1,_TAG_HEADER_BIN_REF)
#define ERL_BIN_REF_SIZE (sizeof(BinRef)/sizeof(Eterm))
#define ERL_REFC_BITS_SIZE (ERL_BIN_REF_SIZE+ERL_SUB_BITS_SIZE)
typedef struct erl_heap_bits {
    Eterm thing_word;
    Uint size;
    Eterm data[1];
} ErlHeapBits;
#define heap_bin_size__(num_bytes)                                            \
  (sizeof(ErlHeapBits)/sizeof(Eterm) - 1 +                                    \
   ((num_bytes) + sizeof(Eterm) - 1)/sizeof(Eterm))
#define heap_bits_size(num_bits)                                              \
    heap_bin_size__(NBYTES(num_bits))
#define header_heap_bits(num_bits) \
  _make_header(heap_bits_size(num_bits)-1,_TAG_HEADER_HEAP_BITS)
#define ERL_ONHEAP_BINARY_LIMIT 64
#define ERL_ONHEAP_BITS_LIMIT (ERL_ONHEAP_BINARY_LIMIT * 8)
#define HEAP_BITSTRING(hp, data, offset, size)                                \
    (ASSERT(size <= ERL_ONHEAP_BITS_LIMIT),                                   \
     (hp)[0] = header_heap_bits(size),                                        \
     (hp)[1] = size,                                                          \
     copy_binary_to_buffer((byte*)&(hp)[2], 0, (byte*)data, offset, size),    \
     make_bitstring(hp))
struct erl_bits_state {
    byte* erts_current_bin;
    Uint erts_bin_offset;
};
typedef struct erl_bits_state ErlBitsState;
#define ERL_BITS_EBS_FROM_REG(Reg)                              \
    ((ErlBitsState *) ((char *)(Reg) +                          \
                       (offsetof(ErtsSchedulerRegisters,        \
                                 aux_regs.d.erl_bits_state) -   \
                        offsetof(ErtsSchedulerRegisters,        \
                                 x_reg_array.d))))
#define WSIZE(n) ((n + sizeof(Eterm) - 1) / sizeof(Eterm))
#define ERL_UNIT_BITS 8
Eterm erts_bs_start_match_2(Process *p, Eterm Bin, Uint Max);
ErlSubBits *erts_bs_start_match_3(Process *p, Eterm Bin);
Eterm erts_bs_get_integer_2(Process *p, Uint num_bits, unsigned flags, ErlSubBits* sb);
Eterm erts_bs_get_float_2(Process *p, Uint num_bits, unsigned flags, ErlSubBits* sb);
Eterm erts_bs_get_binary_2(Process *p, Uint num_bits, ErlSubBits* sb);
Eterm erts_bs_get_binary_all_2(Process *p, ErlSubBits* sb);
int erts_bs_put_integer_be(ErlBitsState *EBS, Eterm Integer, Uint num_bits);
int erts_bs_put_integer_le(ErlBitsState *EBS, Eterm Integer, Uint num_bits);
#if !defined(BEAMASM)
int erts_bs_put_utf8(ErlBitsState *EBS, Eterm Integer);
#endif
int erts_bs_put_utf16(ErlBitsState *EBS, Eterm Integer, Uint flags);
int erts_bs_put_binary(ErlBitsState *EBS, Process *c_p, Eterm Bin, Uint num_bits);
int erts_bs_put_binary_all(ErlBitsState* EBS, Process *c_p, Eterm Bin, Uint unit);
Eterm erts_bs_put_float(ErlBitsState *EBS, Process *c_p, Eterm Float,
                        Uint num_bits, int flags);
void erts_bs_put_string(ErlBitsState *EBS, byte* iptr, Uint num_bytes);
Uint32 erts_bs_get_unaligned_uint32(ErlSubBits* sb);
Eterm erts_bs_get_utf8(ErlSubBits* sb);
Eterm erts_bs_get_utf16(ErlSubBits* sb, Uint flags);
Eterm erts_bs_append_checked(Process* p, Eterm* reg, Uint live, Uint size,
                             Uint extra_words, Uint unit);
Eterm erts_bs_private_append_checked(ErlBitsState* EBS, Process* p,
                                     Eterm bin, Uint size);
Eterm erts_bs_init_writable(Process* p, Eterm sz);
ERTS_GLB_INLINE void
copy_binary_to_buffer(byte *dst_base, Uint dst_offset,
                      const byte *src_base, Uint src_offset,
                      Uint size);
void erts_copy_bits_fwd(const byte* src, size_t soffs,
                        byte* dst, size_t doffs, size_t n);
void erts_copy_bits_rev(const byte* src, size_t soffs,
                        byte* dst, size_t doffs, size_t n);
ERTS_GLB_INLINE int erts_cmp_bits(const byte* a_ptr,
                                  Uint a_offs,
                                  const byte* b_ptr,
                                  Uint b_offs,
                                  Uint size);
int erts_cmp_bits__(const byte* a_ptr,
                    Uint a_offs,
                    const byte* b_ptr,
                    Uint b_offs,
                    Uint size);
void erts_pin_writable_binary(ErlSubBits *sb, BinRef *br);
ERTS_GLB_INLINE Uint erts_extracted_bitstring_size(Uint size);
#define BUILD_SUB_BITSTRING_HEAP_NEED \
    (MAX(ERL_SUB_BITS_SIZE, heap_bits_size(ERL_ONHEAP_BITS_LIMIT)))
Eterm erts_build_sub_bitstring(Eterm **hp,
                               Eterm br_flags,
                               const BinRef *br,
                               const byte *base,
                               Uint offset,
                               Uint size);
Eterm erts_make_sub_bitstring(Process *p, Eterm bitstring, Uint offset, Uint size);
Eterm erts_make_sub_binary(Process *p, Eterm bitstring, Uint offset, Uint size);
Eterm erts_new_bitstring(Process *p, Uint size, byte **datap);
Eterm erts_new_bitstring_refc(Process *p, Uint size, Binary **binp, byte **datap);
Eterm erts_new_bitstring_from_data(Process *p, Uint size, const byte *data);
Eterm erts_new_binary(Process *p, Uint size, byte **datap);
Eterm erts_new_binary_refc(Process *p, Uint size, Binary **binp, byte **datap);
Eterm erts_new_binary_from_data(Process *p, Uint size, const byte *data);
Eterm erts_hfact_new_bitstring(ErtsHeapFactory *hfact,
                               Uint extra,
                               Uint size,
                               byte **datap);
Eterm erts_hfact_new_binary_from_data(ErtsHeapFactory *hfact,
                                      Uint extra,
                                      Uint size,
                                      const byte *data);
Eterm erts_wrap_refc_bitstring(struct erl_off_heap_header **oh,
                               Uint64 *overhead,
                               Eterm **hpp,
                               Binary *bin,
                               byte *data,
                               Uint offset,
                               Uint size);
#define ERTS_BR_OVERHEAD(oh, br)                                              \
    do {                                                                      \
        (oh)->overhead += ((br)->val)->orig_size / sizeof(Eterm);             \
    } while(0)
#define ERTS_GET_BITSTRING(Bin,                                               \
                           Base,                                              \
                           Offset,                                            \
                           Size)                                              \
    do {                                                                      \
        ERTS_DECLARE_DUMMY(const BinRef *_unused_br);                         \
        ERTS_DECLARE_DUMMY(Eterm _unused_br_tag);                             \
        ERTS_GET_BITSTRING_REF(Bin,                                           \
                               _unused_br_tag,                                \
                               _unused_br,                                    \
                               Base,                                          \
                               Offset,                                        \
                               Size);                                         \
    } while (0)
#define ERTS_GET_BITSTRING_REF(Bin, RefFlags, Ref, Base, Offset, Size)        \
    do {                                                                      \
        const Eterm *_unboxed = bitstring_val(Bin);                           \
        if (*_unboxed == HEADER_SUB_BITS) {                                   \
            ErlSubBits* _sb = (ErlSubBits*)_unboxed;                          \
            BinRef *_br;                                                      \
            _unboxed = boxed_val(_sb->orig);                                  \
                          \
            _br = (*_unboxed == HEADER_BIN_REF) ? (BinRef*)_unboxed : NULL;   \
            ASSERT(erl_sub_bits_is_match_context(_sb) || _br);                \
            if (ERTS_LIKELY(!erl_sub_bits_was_writable(_sb))) {               \
                Base = (byte*)(_sb->base_flags & ~ERL_SUB_BITS_FLAG_MASK);    \
            } else {                                                          \
                    \
                   \
                                               \
                ASSERT(_br);                                                  \
                Base = (byte*)((_br->val)->orig_bytes);                       \
                if (!((_br->val)->intern.flags & BIN_FLAG_WRITABLE)) {        \
                    _sb->base_flags = (UWord)Base;                            \
                }                                                             \
            }                                                                 \
            Offset = _sb->start;                                              \
            Size = _sb->end - _sb->start;                                     \
            RefFlags = _sb->orig & TAG_PTR_MASK__;                            \
            Ref = _br;                                                        \
        } else {                                                              \
            const ErlHeapBits *_hb = ((ErlHeapBits*)_unboxed);                \
            Base = (byte*)&_hb->data[0];                                      \
            Size = _hb->size;                                                 \
            Offset = 0;                                                       \
            RefFlags = 0;                                                     \
            Ref = NULL;                                                       \
        }                                                                     \
    } while (0)
#define ERTS_PIN_BITSTRING(Bin, RefFlags, Ref, Base, Offset, Size)            \
    do {                                                                      \
        const Eterm *_unboxed = bitstring_val(Bin);                           \
        if (*_unboxed == HEADER_SUB_BITS) {                                   \
            ErlSubBits* _sb = (ErlSubBits*)_unboxed;                          \
            BinRef *_br;                                                      \
            _unboxed = boxed_val(_sb->orig);                                  \
                          \
            _br = (*_unboxed == HEADER_BIN_REF) ? (BinRef*)_unboxed : NULL;   \
            ASSERT(erl_sub_bits_is_match_context(_sb) || _br);                \
            if (ERTS_LIKELY(_br != NULL)) {                                   \
                          \
                erts_pin_writable_binary(_sb, _br);                           \
            }                                                                 \
            Base = (byte*)(_sb->base_flags & ~ERL_SUB_BITS_FLAG_MASK);        \
            Offset = _sb->start;                                              \
            Size = _sb->end - _sb->start;                                     \
            RefFlags = _sb->orig & TAG_PTR_MASK__;                            \
            Ref = _br;                                                        \
        } else {                                                              \
            const ErlHeapBits *_hb = ((ErlHeapBits*)_unboxed);                \
            Base = (byte*)&_hb->data[0];                                      \
            Size = _hb->size;                                                 \
            Offset = 0;                                                       \
            Ref = NULL;                                                       \
        }                                                                     \
    } while (0)
ERTS_GLB_INLINE const byte*
erts_get_aligned_binary_bytes_extra(Eterm bin,
                                    Uint *size_ptr,
                                    const byte **base_ptr,
                                    ErtsAlcType_t allocator,
                                    Uint extra);
ERTS_GLB_INLINE const byte*
erts_get_aligned_binary_bytes(Eterm bin,
                              Uint *size_ptr,
                              const byte** base_ptr);
ERTS_GLB_INLINE void
erts_free_aligned_binary_bytes_extra(const byte* buf, ErtsAlcType_t);
ERTS_GLB_INLINE void
erts_free_aligned_binary_bytes(const byte* buf);
#define BSF_ALIGNED 1
#define BSF_LITTLE 2
#define BSF_SIGNED 4
#define BSF_EXACT 8
#define BSF_NATIVE 16
#define BSC_APPEND              0
#define BSC_PRIVATE_APPEND      1
#define BSC_BINARY              2
#define BSC_BINARY_FIXED_SIZE   3
#define BSC_BINARY_ALL          4
#define BSC_FLOAT               5
#define BSC_FLOAT_FIXED_SIZE    6
#define BSC_INTEGER             7
#define BSC_INTEGER_FIXED_SIZE  8
#define BSC_STRING              9
#define BSC_UTF8               10
#define BSC_UTF16              11
#define BSC_UTF32              12
#define BSC_NUM_ARGS            5
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE void
erl_sub_bits_init(ErlSubBits *sb, UWord flags, Eterm orig, const void *base,
                  Uint offset, Uint size)
{
    Uint adjustment = (UWord)base & ERL_SUB_BITS_FLAG_MASK;
    ASSERT(is_boxed(orig));
    ASSERT(!(flags & ~ERL_SUB_BITS_FLAG_MASK));
#ifdef DEBUG
    if (*boxed_val(orig) == HEADER_BIN_REF) {
        Binary *bin = ((BinRef*)boxed_val(orig))->val;
        ASSERT((flags & ERL_SUB_BITS_FLAG_VOLATILE) ||
               !(bin->intern.flags & BIN_FLAG_WRITABLE));
    } else {
        ASSERT(flags == ERL_SUB_BITS_FLAGS_MATCH_CONTEXT);
    }
#endif
    sb->thing_word = HEADER_SUB_BITS;
    sb->start = offset + adjustment * 8;
    sb->end = sb->start + size;
    sb->base_flags = ((UWord)base - adjustment) | flags;
    sb->orig = orig;
}
ERTS_GLB_INLINE void
erl_sub_bits_update_moved(ErlSubBits *sb, Eterm orig)
{
    Eterm *ptr = ptr_val(orig);
    if (thing_subtag(*ptr) == HEAP_BITS_SUBTAG) {
        UWord new_base = (UWord)&((ErlHeapBits*)ptr)->data;
        ASSERT(erl_sub_bits_is_match_context(sb));
        ASSERT(!(new_base & ERL_SUB_BITS_FLAG_MASK));
        sb->base_flags = new_base | (sb->base_flags & ERL_SUB_BITS_FLAG_MASK);
    }
}
ERTS_GLB_INLINE void
copy_binary_to_buffer(byte *dst_base, Uint dst_offset,
                      const byte *src_base, Uint src_offset,
                      Uint size)
{
    if (size > 0) {
        dst_base += BYTE_OFFSET(dst_offset);
        src_base += BYTE_OFFSET(src_offset);
        if (((dst_offset | src_offset | size) & 7) == 0) {
            sys_memcpy(dst_base, src_base, BYTE_SIZE(size));
        } else {
            erts_copy_bits_fwd(src_base, BIT_OFFSET(src_offset),
                               dst_base, BIT_OFFSET(dst_offset),
                               size);
        }
    }
}
ERTS_GLB_INLINE int
erts_cmp_bits(const byte* a_ptr,
              Uint a_offs,
              const byte* b_ptr,
              Uint b_offs,
              Uint size)
{
    if (size > 0) {
        a_ptr += BYTE_OFFSET(a_offs);
        b_ptr += BYTE_OFFSET(b_offs);
        if (((a_offs | b_offs | size) & 7) == 0) {
            return sys_memcmp(a_ptr, b_ptr, BYTE_SIZE(size));
        }
        return erts_cmp_bits__(a_ptr,
                               BIT_OFFSET(a_offs),
                               b_ptr,
                               BIT_OFFSET(b_offs),
                               size);
    }
    return 0;
}
ERTS_GLB_INLINE Uint
erts_extracted_bitstring_size(Uint size)
{
    if (size <= ERL_ONHEAP_BITS_LIMIT) {
        return heap_bits_size(size);
    } else {
        ERTS_CT_ASSERT(ERL_SUB_BITS_SIZE <= ERL_ONHEAP_BINARY_LIMIT);
        return ERL_SUB_BITS_SIZE;
    }
}
ERTS_GLB_INLINE const byte*
erts_get_aligned_binary_bytes(Eterm bin, Uint *size_ptr, const byte **base_ptr)
{
    return erts_get_aligned_binary_bytes_extra(bin,
                                               size_ptr,
                                               base_ptr,
                                               ERTS_ALC_T_TMP,
                                               0);
}
ERTS_GLB_INLINE const byte*
erts_get_aligned_binary_bytes_extra(Eterm bin,
                                    Uint *size_ptr,
                                    const byte **base_ptr,
                                    ErtsAlcType_t allocator,
                                    Uint extra)
{
    if (is_bitstring(bin)) {
        Uint offset, size;
        const byte *base;
        ERTS_GET_BITSTRING(bin, base, offset, size);
        if (TAIL_BITS(size) == 0) {
            *size_ptr = BYTE_SIZE(size);
            if (BIT_OFFSET(offset) != 0) {
                byte *bytes = (byte*)erts_alloc(allocator,
                                                NBYTES(size) + extra);
                *base_ptr = bytes;
                erts_copy_bits_fwd(base, offset, &bytes[extra], 0, size);
                return &bytes[extra];
            }
            return &base[BYTE_OFFSET(offset)];
        }
    }
    return NULL;
}
ERTS_GLB_INLINE void
erts_free_aligned_binary_bytes_extra(const byte *bytes, ErtsAlcType_t allocator)
{
    if (bytes) {
        erts_free(allocator, (void*)bytes);
    }
}
ERTS_GLB_INLINE void
erts_free_aligned_binary_bytes(const byte *bytes)
{
    erts_free_aligned_binary_bytes_extra(bytes, ERTS_ALC_T_TMP);
}
#endif
#endif