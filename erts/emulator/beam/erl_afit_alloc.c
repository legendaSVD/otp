#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "global.h"
#define GET_ERL_AF_ALLOC_IMPL
#include "erl_afit_alloc.h"
struct AFFreeBlock_t_ {
    Block_t block_head;
    AFFreeBlock_t *prev;
    AFFreeBlock_t *next;
};
#define AF_BLK_SZ(B) MBC_FBLK_SZ(&(B)->block_head)
#define MIN_MBC_SZ		(16*1024)
static Block_t *	get_free_block		(Allctr_t *, Uint, Block_t *, Uint);
static void		link_free_block		(Allctr_t *, Block_t *);
static void		unlink_free_block	(Allctr_t *, Block_t *);
static Eterm		info_options		(Allctr_t *, char *, fmtfn_t *,
						 void *arg, Uint **, Uint *);
static void		init_atoms		(void);
static int atoms_initialized = 0;
void
erts_afalc_init(void)
{
    atoms_initialized = 0;
}
Allctr_t *
erts_afalc_start(AFAllctr_t *afallctr,
		 AFAllctrInit_t *afinit,
		 AllctrInit_t *init)
{
    struct {
	int dummy;
	AFAllctr_t allctr;
    } zero = {0};
    Allctr_t *allctr = (Allctr_t *) afallctr;
    sys_memcpy((void *) afallctr, (void *) &zero.allctr, sizeof(AFAllctr_t));
    allctr->mbc_header_size		= sizeof(Carrier_t);
    allctr->min_mbc_size		= MIN_MBC_SZ;
    allctr->min_block_size		= sizeof(AFFreeBlock_t);
    allctr->vsn_str			= ERTS_ALC_AF_ALLOC_VSN_STR;
    allctr->get_free_block		= get_free_block;
    allctr->link_free_block		= link_free_block;
    allctr->unlink_free_block		= unlink_free_block;
    allctr->info_options		= info_options;
    allctr->get_next_mbc_size		= NULL;
    allctr->creating_mbc		= NULL;
    allctr->destroying_mbc		= NULL;
    allctr->add_mbc                     = NULL;
    allctr->remove_mbc                  = NULL;
    allctr->largest_fblk_in_mbc         = NULL;
    allctr->first_fblk_in_mbc           = NULL;
    allctr->next_fblk_in_mbc            = NULL;
    allctr->init_atoms			= init_atoms;
#ifdef ERTS_ALLOC_UTIL_HARD_DEBUG
    allctr->check_block			= NULL;
    allctr->check_mbc			= NULL;
#endif
    allctr->atoms_initialized		= 0;
    if (!erts_alcu_start(allctr, init))
	return NULL;
    return allctr;
}
static Block_t *
get_free_block(Allctr_t *allctr, Uint size, Block_t *cand_blk, Uint cand_size)
{
    AFAllctr_t *afallctr = (AFAllctr_t *) allctr;
    ASSERT(!cand_blk || cand_size >= size);
    if (afallctr->free_list && AF_BLK_SZ(afallctr->free_list) >= size) {
	AFFreeBlock_t *res = afallctr->free_list;
	afallctr->free_list = res->next;
	if (res->next)
	    res->next->prev = NULL;
	return (Block_t *) res;
    }
    else
	return NULL;
}
static void
link_free_block(Allctr_t *allctr, Block_t *block)
{
    AFFreeBlock_t *blk = (AFFreeBlock_t *) block;
    AFAllctr_t *afallctr = (AFAllctr_t *) allctr;
    if (afallctr->free_list && AF_BLK_SZ(afallctr->free_list) > AF_BLK_SZ(blk)) {
	blk->next = afallctr->free_list->next;
	blk->prev = afallctr->free_list;
	afallctr->free_list->next = blk;
    }
    else {
	blk->next = afallctr->free_list;
	blk->prev = NULL;
	afallctr->free_list = blk;
    }
    if (blk->next)
	blk->next->prev = blk;
}
static void
unlink_free_block(Allctr_t *allctr, Block_t *block)
{
    AFFreeBlock_t *blk = (AFFreeBlock_t *) block;
    AFAllctr_t *afallctr = (AFAllctr_t *) allctr;
    if (blk->prev)
	blk->prev->next = blk->next;
    else
	afallctr->free_list = blk->next;
    if (blk->next)
	blk->next->prev = blk->prev;
}
static struct {
    Eterm as;
    Eterm af;
#ifdef DEBUG
    Eterm end_of_atoms;
#endif
} am;
static void ERTS_INLINE atom_init(Eterm *atom, char *name)
{
    *atom = am_atom_put(name, sys_strlen(name));
}
#define AM_INIT(AM) atom_init(&am.AM, #AM)
static void
init_atoms(void)
{
#ifdef DEBUG
    Eterm *atom;
#endif
    if (atoms_initialized)
	return;
#ifdef DEBUG
    for (atom = (Eterm *) &am; atom <= &am.end_of_atoms; atom++) {
	*atom = THE_NON_VALUE;
    }
#endif
    AM_INIT(as);
    AM_INIT(af);
#ifdef DEBUG
    for (atom = (Eterm *) &am; atom < &am.end_of_atoms; atom++) {
	ASSERT(*atom != THE_NON_VALUE);
    }
#endif
    atoms_initialized = 1;
}
#define bld_uint	erts_bld_uint
#define bld_cons	erts_bld_cons
#define bld_tuple	erts_bld_tuple
static ERTS_INLINE void
add_2tup(Uint **hpp, Uint *szp, Eterm *lp, Eterm el1, Eterm el2)
{
    *lp = bld_cons(hpp, szp, bld_tuple(hpp, szp, 2, el1, el2), *lp);
}
static Eterm
info_options(Allctr_t *allctr,
	     char *prefix,
	     fmtfn_t *print_to_p,
	     void *print_to_arg,
	     Uint **hpp,
	     Uint *szp)
{
    Eterm res = THE_NON_VALUE;
    if (print_to_p) {
	erts_print(*print_to_p, print_to_arg, "%sas: af\n", prefix);
    }
    if (hpp || szp) {
	if (!atoms_initialized)
	    erts_exit(ERTS_ERROR_EXIT, "%s:%d: Internal error: Atoms not initialized",
		     __FILE__, __LINE__);;
	res = NIL;
	add_2tup(hpp, szp, &res, am.as, am.af);
    }
    return res;
}
UWord
erts_afalc_test(UWord op, UWord a1, UWord a2)
{
    switch (op) {
    default:	ASSERT(0); return ~((UWord) 0);
    }
}