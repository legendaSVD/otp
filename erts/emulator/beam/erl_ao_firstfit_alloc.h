#ifndef ERL_AO_FIRSTFIT_ALLOC__
#define ERL_AO_FIRSTFIT_ALLOC__
#include "erl_alloc_util.h"
#define ERTS_ALC_AOFF_ALLOC_VSN_STR "0.9"
typedef struct AOFFAllctr_t_ AOFFAllctr_t;
enum AOFFSortOrder {
    FF_AGEFF = 0,
    FF_AOFF  = 1,
    FF_AOBF  = 2,
    FF_BF    = 3,
    FF_CHAOS = -1
};
typedef struct {
    enum AOFFSortOrder blk_order;
    enum AOFFSortOrder crr_order;
} AOFFAllctrInit_t;
#define ERTS_DEFAULT_AOFF_ALLCTR_INIT {0}
void erts_aoffalc_init(void);
Allctr_t *erts_aoffalc_start(AOFFAllctr_t *, AOFFAllctrInit_t*, AllctrInit_t *);
#endif
#if defined(GET_ERL_AOFF_ALLOC_IMPL) && !defined(ERL_AOFF_ALLOC_IMPL__)
#define ERL_AOFF_ALLOC_IMPL__
#define GET_ERL_ALLOC_UTIL_IMPL
#include "erl_alloc_util.h"
struct AOFFAllctr_t_ {
    Allctr_t		allctr;
    struct AOFF_RBTree_t_* mbc_root;
    enum AOFFSortOrder blk_order;
    enum AOFFSortOrder crr_order;
};
UWord erts_aoffalc_test(UWord, UWord, UWord);
#endif