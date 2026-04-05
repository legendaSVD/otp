#ifndef __ERL_VM_H__
#define __ERL_VM_H__
#ifdef VALGRIND
#undef NATIVE_ERLANG_STACK
#undef ERLANG_FRAME_POINTERS
#endif
#ifdef CODE_MODEL_SMALL
#undef ERLANG_FRAME_POINTERS
#endif
#if defined(DEBUG) && !defined(CHECK_FOR_HOLES) && !defined(__WIN32__)
# define CHECK_FOR_HOLES
#endif
#define BEAM 1
#define EMULATOR "BEAM"
#define SEQ_TRACE 1
#define CONTEXT_REDS 4000
#define MAX_ARG      255
#define MAX_REG      1024
#define REG_MASK     (MAX_REG - 1)
#define MAX_BIF_ARITY 4
#define ERTS_X_REGS_ALLOCATED (MAX_REG+3)
#define H_DEFAULT_SIZE  233
#define VH_DEFAULT_SIZE  32768
#define H_DEFAULT_MAX_SIZE 0
typedef enum {
    ERTS_FRAME_LAYOUT_RA,
    ERTS_FRAME_LAYOUT_FP_RA
} ErtsFrameLayout;
ERTS_GLB_INLINE
int erts_cp_size(void);
#if defined(BEAMASM) && defined(ERLANG_FRAME_POINTERS)
extern ErtsFrameLayout ERTS_WRITE_UNLIKELY(erts_frame_layout);
#   define CP_SIZE erts_cp_size()
#else
#   define erts_frame_layout ERTS_FRAME_LAYOUT_RA
#   define CP_SIZE 1
#endif
#if defined(BEAMASM) && defined(NATIVE_ERLANG_STACK)
#define S_REDZONE (CP_SIZE * 3)
#elif defined(BEAMASM) && defined(__aarch64__)
#define S_REDZONE (CP_SIZE * 3)
#elif defined(DEBUG)
#define S_REDZONE CP_SIZE
#else
#define S_REDZONE 0
#endif
#define S_RESERVED (CP_SIZE + S_REDZONE)
#define ErtsHAllocLockCheck(P) \
  ERTS_LC_ASSERT(erts_dbg_check_halloc_lock((P)))
#ifdef DEBUG
#  ifdef CHECK_FOR_HOLES
Eterm* erts_set_hole_marker(Eterm* ptr, Uint sz);
#    define INIT_HEAP_MEM(p,sz) erts_set_hole_marker(p, (sz))
#  else
#    define INIT_HEAP_MEM(p,sz) sys_memset(p,0x01,(sz)*sizeof(Eterm*))
#  endif
#else
#  define INIT_HEAP_MEM(p,sz) ((void)0)
#endif
#ifdef FORCE_HEAP_FRAGS
#  define IS_FORCE_HEAP_FRAGS 1
#else
#  define IS_FORCE_HEAP_FRAGS 0
#endif
#define HAllocX(p, sz, xtra)                                                \
  (ASSERT((sz) >= 0),                                                       \
     ErtsHAllocLockCheck(p),                                                \
     ((IS_FORCE_HEAP_FRAGS || (!HEAP_START(p) || HeapWordsLeft(p) < (sz)))  \
      ? erts_heap_alloc((p),(sz),(xtra))                                    \
      : (INIT_HEAP_MEM(HEAP_TOP(p), sz),                                    \
         HEAP_TOP(p) = HEAP_TOP(p) + (sz), HEAP_TOP(p) - (sz))))
#define HAlloc(P, SZ) HAllocX(P,SZ,0)
#define HRelease(p, endp, ptr)					\
  if ((ptr) == (endp)) {					\
     ;								\
  } else if (HEAP_START(p) <= (ptr) && (ptr) < HEAP_TOP(p)) {	\
     ASSERT(HEAP_TOP(p) == (endp));                             \
     HEAP_TOP(p) = (ptr);					\
  } else {							\
     ASSERT(MBUF(p)->mem + MBUF(p)->used_size == (endp));       \
     erts_heap_frag_shrink(p, ptr);                             \
  }
#define HeapWordsLeft(p) (HEAP_LIMIT(p) - HEAP_TOP(p))
#if defined(DEBUG) || defined(CHECK_FOR_HOLES)
#  ifdef ARCH_64
#    define ERTS_HOLE_MARKER \
    make_catch(UWORD_CONSTANT(0xdeadbeaf00000000) >> _TAG_IMMED2_SIZE)
#  else
#    define ERTS_HOLE_MARKER \
    make_catch(UWORD_CONSTANT(0xdead0000) >> _TAG_IMMED2_SIZE)
#  endif
#endif
#ifdef CHECK_FOR_HOLES
# define HeapOnlyAlloc(p, sz)					\
    (ASSERT((sz) >= 0),					        \
     (ASSERT(((HEAP_LIMIT(p) - HEAP_TOP(p)) >= (sz))),	        \
      (erts_set_hole_marker(HEAP_TOP(p), (sz)),			\
       (HEAP_TOP(p) = HEAP_TOP(p) + (sz), HEAP_TOP(p) - (sz)))))
#else
# define HeapOnlyAlloc(p, sz)					\
    (ASSERT((sz) >= 0),					        \
     (ASSERT(((HEAP_LIMIT(p) - HEAP_TOP(p)) >= (sz))),	        \
      (HEAP_TOP(p) = HEAP_TOP(p) + (sz), HEAP_TOP(p) - (sz))))
#endif
#if defined(VALGRIND)
#  define HeapFragOnlyAlloc(p, sz)              \
  (ASSERT((sz) >= 0),                           \
   ErtsHAllocLockCheck(p),                      \
   erts_heap_alloc((p),(sz),0))
#else
#  define HeapFragOnlyAlloc(p, sz)              \
  (ASSERT((sz) >= 0),                           \
   ErtsHAllocLockCheck(p),                      \
   erts_heap_alloc((p),(sz),512))
#endif
typedef struct op_entry {
   char* name;
   Uint32 mask[3];
#ifndef BEAMASM
   unsigned involves_r;
   int sz;
   int adjust;
   char* pack;
#endif
   char* sign;
} OpEntry;
extern const OpEntry opc[];
extern const int num_instructions;
extern Uint erts_instr_count[];
#define ATOM_TEXT_SIZE  32768
#define ITIME 100
#define MAX_PORT_LINK 8
extern int H_MIN_SIZE;
extern int BIN_VH_MIN_SIZE;
extern Uint H_MAX_SIZE;
extern int H_MAX_FLAGS;
extern int erts_atom_table_size;
extern int erts_pd_initial_size;
#define hi_byte(a) ((a) >> 8)
#define lo_byte(a) ((a) & 255)
#define make_16(x, y) (((x) << 8) | (y))
#define make_24(x,y,z) (((x) << 16) | ((y) << 8) | (z))
#define make_32(x3,x2,x1,x0) (((x3)<<24) | ((x2)<<16) | ((x1)<<8) | (x0))
#define make_signed_24(x,y,z) ((sint32) (((x) << 24) | ((y) << 16) | ((z) << 8)) >> 8)
#define make_signed_32(x3,x2,x1,x0) ((sint32) (((x3) << 24) | ((x2) << 16) | ((x1) << 8) | (x0)))
#include "erl_term.h"
#if defined(NO_JUMP_TABLE) || defined(BEAMASM)
#  define BeamOpsAreInitialized() (1)
#  define BeamOpCodeAddr(OpCode) ((BeamInstr)(OpCode))
#else
extern void** beam_ops;
#  define BeamOpsAreInitialized() (beam_ops != 0)
#  define BeamOpCodeAddr(OpCode) ((BeamInstr)beam_ops[(OpCode)])
#endif
#if defined(ARCH_64) && defined(CODE_MODEL_SMALL) && !defined(BEAMASM)
#  define BeamCodeAddr(InstrWord) ((BeamInstr)(Uint32)(InstrWord))
#  define BeamSetCodeAddr(InstrWord, Addr) (((InstrWord) & ~((1ull << 32)-1)) | (Addr))
#  define BeamExtraData(InstrWord) ((InstrWord) >> 32)
#else
#  define BeamCodeAddr(InstrWord) ((BeamInstr)(InstrWord))
#  define BeamSetCodeAddr(InstrWord, Addr) (Addr)
#endif
#define BeamIsOpCode(InstrWord, OpCode) (BeamCodeAddr(InstrWord) == BeamOpCodeAddr(OpCode))
#ifndef BEAMASM
#define BeamIsReturnCallAccTrace(w) \
    BeamIsOpCode(*(const BeamInstr*)(w), op_i_call_trace_return)
#define BeamIsReturnToTrace(w) \
    BeamIsOpCode(*(const BeamInstr*)(w), op_i_return_to_trace)
#define BeamIsReturnTrace(w) \
    BeamIsOpCode(*(const BeamInstr*)(w), op_return_trace)
#else
#define BeamIsReturnCallAccTrace(w) \
    ((w) == beam_call_trace_return)
#define BeamIsReturnToTrace(w) \
    ((w) == beam_return_to_trace)
#define BeamIsReturnTrace(w) \
    ((w) == beam_return_trace || (w) == beam_exception_trace)
#endif
#define BEAM_RETURN_CALL_ACC_TRACE_FRAME_SZ 3
#define BEAM_RETURN_TO_TRACE_FRAME_SZ   1
#define BEAM_RETURN_TRACE_FRAME_SZ      3
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE
int erts_cp_size(void)
{
    if (erts_frame_layout == ERTS_FRAME_LAYOUT_RA) {
        return 1;
    }
    ASSERT(erts_frame_layout == ERTS_FRAME_LAYOUT_FP_RA);
    return 2;
}
#endif
#endif