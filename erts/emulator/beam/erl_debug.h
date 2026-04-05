#ifndef _ERL_DEBUG_H_
#define _ERL_DEBUG_H_
#ifdef DEBUG
#include "erl_term.h"
#define DEBUG_BAD_BYTE 0x01
#define DEBUG_BAD_WORD 0x01010101
#define VERBOSE(flag, format) (flag & verbose ? erts_printf format : 0)
#define DEBUG_DEFAULT      0x0000
#define DEBUG_SYSTEM       0x0001
#define DEBUG_PRIVATE_GC   0x0002
#define DEBUG_ALLOCATION   0x0004
#define DEBUG_MESSAGES     0x0008
#define DEBUG_THREADS      0x0010
#define DEBUG_PROCESSES    0x0020
#define DEBUG_MEMORY       0x0040
#define DEBUG_SHCOPY       0x0080
extern Uint32 verbose;
void upp(byte*, size_t);
void pat(Eterm);
void pinfo(void);
void pp(Process*);
void ppi(Eterm);
void pba(Process*, int);
void td(Eterm);
void ps(Process*, Eterm*);
#undef ERTS_OFFHEAP_DEBUG
#define ERTS_OFFHEAP_DEBUG
#else
#define VERBOSE(flag,format)
#endif
#ifdef ERTS_OFFHEAP_DEBUG
#define ERTS_CHK_OFFHEAP(P) erts_check_off_heap((P))
#define ERTS_CHK_OFFHEAP2(P, HT) erts_check_off_heap2((P), (HT))
void erts_check_off_heap(Process *);
void erts_check_off_heap2(Process *, Eterm *);
#else
#define ERTS_CHK_OFFHEAP(P)
#define ERTS_CHK_OFFHEAP2(P, HT)
#endif
extern void erts_check_off_heap(Process *p);
extern void erts_check_stack(Process *p);
extern void erts_check_heap(Process *p);
extern void erts_check_memory(Process *p, Eterm *start, Eterm *end);
extern void verify_process(Process *p);
extern void print_tagged_memory(Eterm *start, Eterm *end);
extern void print_untagged_memory(Eterm *start, Eterm *end);
extern void print_memory(Process *p);
extern void print_memory_info(Process *p);
#endif