#ifndef ERL_AFIT_ALLOC__
#define ERL_AFIT_ALLOC__
#include "erl_alloc_util.h"
#define ERTS_ALC_AF_ALLOC_VSN_STR "0.9"
typedef struct AFAllctr_t_ AFAllctr_t;
typedef struct {
    int dummy;
} AFAllctrInit_t;
#define ERTS_DEFAULT_AF_ALLCTR_INIT {                                      \
    0					\
}
void erts_afalc_init(void);
Allctr_t *erts_afalc_start(AFAllctr_t *, AFAllctrInit_t *, AllctrInit_t *);
#endif
#if defined(GET_ERL_AF_ALLOC_IMPL) && !defined(ERL_AF_ALLOC_IMPL__)
#define ERL_AF_ALLOC_IMPL__
#define GET_ERL_ALLOC_UTIL_IMPL
#include "erl_alloc_util.h"
typedef struct AFFreeBlock_t_ AFFreeBlock_t;
struct AFAllctr_t_ {
    Allctr_t		allctr;
    AFFreeBlock_t *	free_list;
};
UWord erts_afalc_test(UWord, UWord, UWord);
#endif