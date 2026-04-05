#ifndef _ERL_PROCESS_DICT_H
#define _ERL_PROCESS_DICT_H
#include "sys.h"
#include "erl_term_hashing.h"
typedef struct proc_dict {
    unsigned int sizeMask;
    unsigned int usedSlots;
    unsigned int arraySize;
    unsigned int splitPosition;
    Uint numElements;
    Eterm data[1];
} ProcDict;
#define ERTS_PD_START(PD) ((PD)->data)
#define ERTS_PD_SIZE(PD)  ((PD)->usedSlots)
int erts_pd_set_initial_size(int size);
Uint erts_dicts_mem_size(struct process *p);
void erts_erase_dicts(struct process *p);
void erts_dictionary_dump(fmtfn_t to, void *to_arg, ProcDict *pd);
void erts_deep_dictionary_dump(fmtfn_t to, void *to_arg,
			       ProcDict* pd, void (*cb)(fmtfn_t, void *, Eterm obj));
Eterm erts_dictionary_copy(ErtsHeapFactory *hfact, ProcDict *pd, Uint reserve_size);
Eterm erts_pd_hash_get(struct process *p, Eterm id);
erts_ihash_t erts_pd_make_hx(Eterm key);
Eterm erts_pd_hash_get_with_hx(Process *p, erts_ihash_t hx, Eterm id);
#endif