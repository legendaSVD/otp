#ifndef ERL_MSEG_H_
#define ERL_MSEG_H_
#include "sys.h"
#include "erl_alloc_types.h"
#include "erl_mmap.h"
#ifdef ERTS_HAVE_GENUINE_OS_MMAP
#  define HAVE_ERTS_MSEG 1
#  define ERTS_HAVE_MSEG_SUPER_ALIGNED 1
#else
#  define HAVE_ERTS_MSEG 0
#  define ERTS_HAVE_MSEG_SUPER_ALIGNED 0
#endif
#if ERTS_HAVE_MSEG_SUPER_ALIGNED
#  define MSEG_ALIGN_BITS ERTS_MMAP_SUPERALIGNED_BITS
#endif
#if HAVE_ERTS_MSEG
#define MSEG_ALIGNED_SIZE     (1 << MSEG_ALIGN_BITS)
#define ERTS_MSEG_FLG_NONE    ((Uint)(0))
#define ERTS_MSEG_FLG_2POW    ((Uint)(1 << 0))
#define ERTS_MSEG_VSN_STR "0.9"
typedef struct {
    Uint amcbf;
    Uint rmcbf;
    Uint mcs;
    Uint nos;
    Uint ndai;
    ErtsMMapInit dflt_mmap;
    ErtsMMapInit literal_mmap;
} ErtsMsegInit_t;
#define ERTS_MSEG_INIT_DEFAULT_INITIALIZER				\
{									\
    4*1024*1024,		\
    20,				\
    10,				\
    0,                  	\
    0,                  	\
    ERTS_MMAP_INIT_DEFAULT_INITER,					\
    ERTS_MMAP_INIT_LITERAL_INITER,                                      \
}
typedef struct {
    int  cache;
    int  preserv;
    UWord abs_shrink_th;
    UWord rel_shrink_th;
    int sched_spec;
} ErtsMsegOpt_t;
extern const ErtsMsegOpt_t erts_mseg_default_opt;
void *erts_mseg_alloc(ErtsAlcType_t, UWord *, Uint);
void *erts_mseg_alloc_opt(ErtsAlcType_t, UWord *, Uint, const ErtsMsegOpt_t *);
void  erts_mseg_dealloc(ErtsAlcType_t, void *, UWord, Uint);
void  erts_mseg_dealloc_opt(ErtsAlcType_t, void *, UWord, Uint, const ErtsMsegOpt_t *);
void *erts_mseg_realloc(ErtsAlcType_t, void *, UWord, UWord *, Uint);
void *erts_mseg_realloc_opt(ErtsAlcType_t, void *, UWord, UWord *, Uint, const ErtsMsegOpt_t *);
void  erts_mseg_clear_cache(void);
void  erts_mseg_cache_check(void);
Uint  erts_mseg_no( const ErtsMsegOpt_t *);
Uint  erts_mseg_unit_size(void);
void  erts_mseg_init(ErtsMsegInit_t *init);
void  erts_mseg_late_init(void);
Eterm erts_mseg_info_options(int, fmtfn_t*, void*, Uint **, Uint *);
Eterm erts_mseg_info(int, fmtfn_t *, void*, int, int, Uint **, Uint *);
#endif
UWord erts_mseg_test(UWord, UWord, UWord, UWord);
#endif