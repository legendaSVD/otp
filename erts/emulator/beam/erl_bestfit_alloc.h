#ifndef ERL_BESTFIT_ALLOC__
#define ERL_BESTFIT_ALLOC__
#include "erl_alloc_util.h"
#define ERTS_ALC_BF_ALLOC_VSN_STR "0.9"
#define ERTS_ALC_AOBF_ALLOC_VSN_STR "0.9"
typedef struct BFAllctr_t_ BFAllctr_t;
typedef struct {
    int ao;
} BFAllctrInit_t;
#define ERTS_DEFAULT_BF_ALLCTR_INIT {                                      \
    0					\
}
void erts_bfalc_init(void);
Allctr_t *erts_bfalc_start(BFAllctr_t *, BFAllctrInit_t *, AllctrInit_t *);
#endif
#if defined(GET_ERL_BF_ALLOC_IMPL) && !defined(ERL_BF_ALLOC_IMPL__)
#define ERL_BF_ALLOC_IMPL__
#define GET_ERL_ALLOC_UTIL_IMPL
#include "erl_alloc_util.h"
typedef struct RBTree_t_ RBTree_t;
struct BFAllctr_t_ {
    Allctr_t		allctr;
    RBTree_t *		mbc_root;
    int 		address_order;
};
UWord erts_bfalc_test(UWord, UWord, UWord);
#endif