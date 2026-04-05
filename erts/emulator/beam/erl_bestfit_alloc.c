#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "global.h"
#define GET_ERL_BF_ALLOC_IMPL
#include "erl_bestfit_alloc.h"
#ifdef DEBUG
#if 0
#define HARD_DEBUG
#endif
#else
#undef HARD_DEBUG
#endif
#define MIN_MBC_SZ		(16*1024)
#define TREE_NODE_FLG		(((Uint) 1) << 0)
#define RED_FLG			(((Uint) 1) << 1)
#ifdef HARD_DEBUG
#  define LEFT_VISITED_FLG	(((Uint) 1) << 2)
#  define RIGHT_VISITED_FLG	(((Uint) 1) << 3)
#endif
#define IS_TREE_NODE(N)		(((RBTree_t *) (N))->flags & TREE_NODE_FLG)
#define IS_LIST_ELEM(N)		(!IS_TREE_NODE(((RBTree_t *) (N))))
#define SET_TREE_NODE(N)	(((RBTree_t *) (N))->flags |= TREE_NODE_FLG)
#define SET_LIST_ELEM(N)	(((RBTree_t *) (N))->flags &= ~TREE_NODE_FLG)
#define IS_RED(N)		(((RBTree_t *) (N)) \
				 && ((RBTree_t *) (N))->flags & RED_FLG)
#define IS_BLACK(N)		(!IS_RED(((RBTree_t *) (N))))
#define SET_RED(N)		(((RBTree_t *) (N))->flags |= RED_FLG)
#define SET_BLACK(N)		(((RBTree_t *) (N))->flags &= ~RED_FLG)
#define BF_BLK_SZ(B)            MBC_FBLK_SZ(&(B)->hdr)
#if 1
#define RBT_ASSERT	ASSERT
#else
#define RBT_ASSERT(x)
#endif
#ifdef HARD_DEBUG
static RBTree_t * check_tree(RBTree_t, int, Uint);
#endif
static void tree_delete(Allctr_t *allctr, Block_t *del);
static Block_t *	aobf_get_free_block	(Allctr_t *, Uint,
						 Block_t *, Uint);
static void		aobf_link_free_block	(Allctr_t *, Block_t *);
#define			aobf_unlink_free_block	tree_delete
static Block_t *	bf_get_free_block	(Allctr_t *, Uint,
						 Block_t *, Uint);
static void		bf_link_free_block	(Allctr_t *, Block_t *);
static ERTS_INLINE void	bf_unlink_free_block	(Allctr_t *, Block_t *);
static Eterm		info_options		(Allctr_t *, char *, fmtfn_t *,
						 void *, Uint **, Uint *);
static void		init_atoms		(void);
struct RBTree_t_ {
    Block_t hdr;
    Uint flags;
    RBTree_t *parent;
    RBTree_t *left;
    RBTree_t *right;
};
typedef struct {
    RBTree_t t;
    RBTree_t *next;
} RBTreeList_t;
#define BF_LIST_NEXT(N) (((RBTreeList_t *) (N))->next)
#define BF_LIST_PREV(N) (((RBTreeList_t *) (N))->t.parent)
#ifdef DEBUG
#define DESTROY_TREE_NODE(N)						\
  sys_memset((void *) (((Block_t *) (N)) + 1),				\
	     0xff,							\
	     (sizeof(RBTree_t) - sizeof(Block_t)))
#define DESTROY_LIST_ELEM(N)						\
  sys_memset((void *) (((Block_t *) (N)) + 1),				\
	     0xff,							\
	     (sizeof(RBTreeList_t) - sizeof(Block_t)))
#else
#define DESTROY_TREE_NODE(N)
#define DESTROY_LIST_ELEM(N)
#endif
static int atoms_initialized = 0;
void
erts_bfalc_init(void)
{
    atoms_initialized = 0;
}
Allctr_t *
erts_bfalc_start(BFAllctr_t *bfallctr,
		 BFAllctrInit_t *bfinit,
		 AllctrInit_t *init)
{
    struct {
	int dummy;
	BFAllctr_t allctr;
    } zero = {0};
    Allctr_t *allctr = (Allctr_t *) bfallctr;
    sys_memcpy((void *) bfallctr, (void *) &zero.allctr, sizeof(BFAllctr_t));
    bfallctr->address_order		= bfinit->ao;
    allctr->mbc_header_size		= sizeof(Carrier_t);
    allctr->min_mbc_size		= MIN_MBC_SZ;
    allctr->min_block_size		= (bfinit->ao
					   ? sizeof(RBTree_t)
					   : sizeof(RBTreeList_t));
    allctr->vsn_str			= (bfinit->ao
					   ? ERTS_ALC_AOBF_ALLOC_VSN_STR
					   : ERTS_ALC_BF_ALLOC_VSN_STR);
    if (bfinit->ao) {
	allctr->get_free_block		= aobf_get_free_block;
	allctr->link_free_block		= aobf_link_free_block;
	allctr->unlink_free_block	= aobf_unlink_free_block;
    }
    else {
	allctr->get_free_block		= bf_get_free_block;
	allctr->link_free_block		= bf_link_free_block;
	allctr->unlink_free_block	= bf_unlink_free_block;
    }
    allctr->info_options		= info_options;
    allctr->get_next_mbc_size		= NULL;
    allctr->creating_mbc		= NULL;
    allctr->destroying_mbc		= NULL;
    allctr->add_mbc                     = NULL;
    allctr->remove_mbc		        = NULL;
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
static ERTS_INLINE void
left_rotate(RBTree_t **root, RBTree_t *x)
{
    RBTree_t *y = x->right;
    x->right = y->left;
    if (y->left)
	y->left->parent = x;
    y->parent = x->parent;
    if (!y->parent) {
	RBT_ASSERT(*root == x);
	*root = y;
    }
    else if (x == x->parent->left)
	x->parent->left = y;
    else {
	RBT_ASSERT(x == x->parent->right);
	x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}
static ERTS_INLINE void
right_rotate(RBTree_t **root, RBTree_t *x)
{
    RBTree_t *y = x->left;
    x->left = y->right;
    if (y->right)
	y->right->parent = x;
    y->parent = x->parent;
    if (!y->parent) {
	RBT_ASSERT(*root == x);
	*root = y;
    }
    else if (x == x->parent->right)
	x->parent->right = y;
    else {
	RBT_ASSERT(x == x->parent->left);
	x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
}
static ERTS_INLINE void
replace(RBTree_t **root, RBTree_t *x, RBTree_t *y)
{
    if (!x->parent) {
	RBT_ASSERT(*root == x);
	*root = y;
    }
    else if (x == x->parent->left)
	x->parent->left = y;
    else {
	RBT_ASSERT(x == x->parent->right);
	x->parent->right = y;
    }
    if (x->left) {
	RBT_ASSERT(x->left->parent == x);
	x->left->parent = y;
    }
    if (x->right) {
	RBT_ASSERT(x->right->parent == x);
	x->right->parent = y;
    }
    y->flags	= x->flags;
    y->parent	= x->parent;
    y->right	= x->right;
    y->left	= x->left;
    DESTROY_TREE_NODE(x);
}
static void
tree_insert_fixup(RBTree_t **root, RBTree_t *blk)
{
    RBTree_t *x = blk, *y;
    RBT_ASSERT(x != *root && IS_RED(x->parent));
    do {
	RBT_ASSERT(IS_RED(x));
	RBT_ASSERT(IS_BLACK(x->parent->parent));
	RBT_ASSERT(x->parent->parent);
	if (x->parent == x->parent->parent->left) {
	    y = x->parent->parent->right;
	    if (IS_RED(y)) {
		SET_BLACK(y);
		x = x->parent;
		SET_BLACK(x);
		x = x->parent;
		SET_RED(x);
	    }
	    else {
		if (x == x->parent->right) {
		    x = x->parent;
		    left_rotate(root, x);
		}
		RBT_ASSERT(x == x->parent->parent->left->left);
		RBT_ASSERT(IS_RED(x));
		RBT_ASSERT(IS_RED(x->parent));
		RBT_ASSERT(IS_BLACK(x->parent->parent));
		RBT_ASSERT(IS_BLACK(y));
		SET_BLACK(x->parent);
		SET_RED(x->parent->parent);
		right_rotate(root, x->parent->parent);
		RBT_ASSERT(x == x->parent->left);
		RBT_ASSERT(IS_RED(x));
		RBT_ASSERT(IS_RED(x->parent->right));
		RBT_ASSERT(IS_BLACK(x->parent));
		break;
	    }
	}
	else {
	    RBT_ASSERT(x->parent == x->parent->parent->right);
	    y = x->parent->parent->left;
	    if (IS_RED(y)) {
		SET_BLACK(y);
		x = x->parent;
		SET_BLACK(x);
		x = x->parent;
		SET_RED(x);
	    }
	    else {
		if (x == x->parent->left) {
		    x = x->parent;
		    right_rotate(root, x);
		}
		RBT_ASSERT(x == x->parent->parent->right->right);
		RBT_ASSERT(IS_RED(x));
		RBT_ASSERT(IS_RED(x->parent));
		RBT_ASSERT(IS_BLACK(x->parent->parent));
		RBT_ASSERT(IS_BLACK(y));
		SET_BLACK(x->parent);
		SET_RED(x->parent->parent);
		left_rotate(root, x->parent->parent);
		RBT_ASSERT(x == x->parent->right);
		RBT_ASSERT(IS_RED(x));
		RBT_ASSERT(IS_RED(x->parent->left));
		RBT_ASSERT(IS_BLACK(x->parent));
		break;
	    }
	}
    } while (x != *root && IS_RED(x->parent));
    SET_BLACK(*root);
}
static void
tree_delete(Allctr_t *allctr, Block_t *del)
{
    BFAllctr_t *bfallctr = (BFAllctr_t *) allctr;
    Uint spliced_is_black;
    RBTree_t **root = &bfallctr->mbc_root;
    RBTree_t *x, *y, *z = (RBTree_t *) del;
    RBTree_t null_x;
    null_x.parent = NULL;
#ifdef HARD_DEBUG
    check_tree(*root, bfallctr->address_order, 0);
#endif
    if (!z->left || !z->right)
	y = z;
    else
	for (y = z->right; y->left; y = y->left)
            ;
    x = y->left ? y->left : y->right;
    spliced_is_black = IS_BLACK(y);
    if (x) {
	x->parent = y->parent;
    }
    else if (!x && spliced_is_black) {
	x = &null_x;
	x->flags = 0;
	SET_BLACK(x);
	x->right = x->left = NULL;
	x->parent = y->parent;
	y->left = x;
    }
    if (!y->parent) {
	RBT_ASSERT(*root == y);
	*root = x;
    }
    else if (y == y->parent->left)
	y->parent->left = x;
    else {
	RBT_ASSERT(y == y->parent->right);
	y->parent->right = x;
    }
    if (y != z) {
	replace(root, z, y);
    }
    if (spliced_is_black) {
	while (IS_BLACK(x) && x->parent) {
	    if (x == x->parent->left) {
		y = x->parent->right;
		RBT_ASSERT(y);
		if (IS_RED(y)) {
		    RBT_ASSERT(y->right);
		    RBT_ASSERT(y->left);
		    SET_BLACK(y);
		    RBT_ASSERT(IS_BLACK(x->parent));
		    SET_RED(x->parent);
		    left_rotate(root, x->parent);
		    y = x->parent->right;
		}
		RBT_ASSERT(y);
		RBT_ASSERT(IS_BLACK(y));
		if (IS_BLACK(y->left) && IS_BLACK(y->right)) {
		    SET_RED(y);
		    x = x->parent;
		}
		else {
		    if (IS_BLACK(y->right)) {
			SET_BLACK(y->left);
			SET_RED(y);
			right_rotate(root, y);
			y = x->parent->right;
		    }
		    RBT_ASSERT(y);
		    if (IS_RED(x->parent)) {
			SET_BLACK(x->parent);
			SET_RED(y);
		    }
		    RBT_ASSERT(y->right);
		    SET_BLACK(y->right);
		    left_rotate(root, x->parent);
		    x = *root;
		    break;
		}
	    }
	    else {
		RBT_ASSERT(x == x->parent->right);
		y = x->parent->left;
		RBT_ASSERT(y);
		if (IS_RED(y)) {
		    RBT_ASSERT(y->right);
		    RBT_ASSERT(y->left);
		    SET_BLACK(y);
		    RBT_ASSERT(IS_BLACK(x->parent));
		    SET_RED(x->parent);
		    right_rotate(root, x->parent);
		    y = x->parent->left;
		}
		RBT_ASSERT(y);
		RBT_ASSERT(IS_BLACK(y));
		if (IS_BLACK(y->right) && IS_BLACK(y->left)) {
		    SET_RED(y);
		    x = x->parent;
		}
		else {
		    if (IS_BLACK(y->left)) {
			SET_BLACK(y->right);
			SET_RED(y);
			left_rotate(root, y);
			y = x->parent->left;
		    }
		    RBT_ASSERT(y);
		    if (IS_RED(x->parent)) {
			SET_BLACK(x->parent);
			SET_RED(y);
		    }
		    RBT_ASSERT(y->left);
		    SET_BLACK(y->left);
		    right_rotate(root, x->parent);
		    x = *root;
		    break;
		}
	    }
	}
	SET_BLACK(x);
	if (null_x.parent) {
	    if (null_x.parent->left == &null_x)
		null_x.parent->left = NULL;
	    else {
		RBT_ASSERT(null_x.parent->right == &null_x);
		null_x.parent->right = NULL;
	    }
	    RBT_ASSERT(!null_x.left);
	    RBT_ASSERT(!null_x.right);
	}
	else if (*root == &null_x) {
	    *root = NULL;
	    RBT_ASSERT(!null_x.left);
	    RBT_ASSERT(!null_x.right);
	}
    }
    DESTROY_TREE_NODE(del);
#ifdef HARD_DEBUG
    check_tree(root, bfallctr->address_order, 0);
#endif
}
static void
aobf_link_free_block(Allctr_t *allctr, Block_t *block)
{
    BFAllctr_t *bfallctr = (BFAllctr_t *) allctr;
    RBTree_t **root = &bfallctr->mbc_root;
    RBTree_t *blk = (RBTree_t *) block;
    Uint blk_sz = BF_BLK_SZ(blk);
    blk->flags	= 0;
    blk->left	= NULL;
    blk->right	= NULL;
    if (!*root) {
	blk->parent = NULL;
	SET_BLACK(blk);
	*root = blk;
    }
    else {
	RBTree_t *x = *root;
	while (1) {
	    Uint size;
	    size = BF_BLK_SZ(x);
	    if (blk_sz < size || (blk_sz == size && blk < x)) {
		if (!x->left) {
		    blk->parent = x;
		    x->left = blk;
		    break;
		}
		x = x->left;
	    }
	    else {
		if (!x->right) {
		    blk->parent = x;
		    x->right = blk;
		    break;
		}
		x = x->right;
	    }
	}
	RBT_ASSERT(blk->parent);
	SET_RED(blk);
	if (IS_RED(blk->parent))
	    tree_insert_fixup(root, blk);
    }
#ifdef HARD_DEBUG
    check_tree(root, 1, 0);
#endif
}
#if 0
static void
aobf_unlink_free_block(Allctr_t *allctr, Block_t *block)
{
    tree_delete(allctr, block);
}
#endif
static Block_t *
aobf_get_free_block(Allctr_t *allctr, Uint size,
		    Block_t *cand_blk, Uint cand_size)
{
    BFAllctr_t *bfallctr = (BFAllctr_t *) allctr;
    RBTree_t **root = &bfallctr->mbc_root;
    RBTree_t *x = *root;
    RBTree_t *blk = NULL;
    Uint blk_sz;
    ASSERT(!cand_blk || cand_size >= size);
    while (x) {
	blk_sz = BF_BLK_SZ(x);
	if (blk_sz < size) {
	    x = x->right;
	}
	else {
	    blk = x;
	    x = x->left;
	}
    }
    if (!blk)
	return NULL;
#ifdef HARD_DEBUG
    ASSERT(blk == check_tree(root, 1, size));
#endif
    if (cand_blk) {
	blk_sz = BF_BLK_SZ(blk);
	if (cand_size < blk_sz)
	    return NULL;
	if (cand_size == blk_sz && ((void *) cand_blk) < ((void *) blk))
	    return NULL;
    }
    aobf_unlink_free_block(allctr, (Block_t *) blk);
    return (Block_t *) blk;
}
static void
bf_link_free_block(Allctr_t *allctr, Block_t *block)
{
    BFAllctr_t *bfallctr = (BFAllctr_t *) allctr;
    RBTree_t **root = &bfallctr->mbc_root;
    RBTree_t *blk = (RBTree_t *) block;
    Uint blk_sz = BF_BLK_SZ(blk);
    SET_TREE_NODE(blk);
    blk->flags	= 0;
    blk->left	= NULL;
    blk->right	= NULL;
    if (!*root) {
	blk->parent = NULL;
	SET_BLACK(blk);
	*root = blk;
    }
    else {
	RBTree_t *x = *root;
	while (1) {
	    Uint size;
	    size = BF_BLK_SZ(x);
	    if (blk_sz == size) {
		SET_LIST_ELEM(blk);
		BF_LIST_NEXT(blk) = BF_LIST_NEXT(x);
		BF_LIST_PREV(blk) = x;
		if (BF_LIST_NEXT(x))
		    BF_LIST_PREV(BF_LIST_NEXT(x)) = blk;
		BF_LIST_NEXT(x) = blk;
		return;
	    }
	    else if (blk_sz < size) {
		if (!x->left) {
		    blk->parent = x;
		    x->left = blk;
		    break;
		}
		x = x->left;
	    }
	    else {
		if (!x->right) {
		    blk->parent = x;
		    x->right = blk;
		    break;
		}
		x = x->right;
	    }
	}
	RBT_ASSERT(blk->parent);
	SET_RED(blk);
	if (IS_RED(blk->parent))
	    tree_insert_fixup(root, blk);
    }
    SET_TREE_NODE(blk);
    BF_LIST_NEXT(blk) = NULL;
#ifdef HARD_DEBUG
    check_tree(root, 0, 0);
#endif
}
static ERTS_INLINE void
bf_unlink_free_block(Allctr_t *allctr, Block_t *block)
{
    BFAllctr_t *bfallctr = (BFAllctr_t *) allctr;
    RBTree_t **root = &bfallctr->mbc_root;
    RBTree_t *x = (RBTree_t *) block;
    if (IS_LIST_ELEM(x)) {
	ASSERT(BF_LIST_PREV(x));
	BF_LIST_NEXT(BF_LIST_PREV(x)) = BF_LIST_NEXT(x);
	if (BF_LIST_NEXT(x))
	    BF_LIST_PREV(BF_LIST_NEXT(x)) = BF_LIST_PREV(x);
    }
    else if (BF_LIST_NEXT(x)) {
	ASSERT(BF_BLK_SZ(BF_LIST_NEXT(x)) == BF_BLK_SZ(x));
	ASSERT(IS_TREE_NODE(x));
	ASSERT(IS_LIST_ELEM(BF_LIST_NEXT(x)));
#ifdef HARD_DEBUG
	check_tree(root, 0, 0);
#endif
	replace(root, x, BF_LIST_NEXT(x));
#ifdef HARD_DEBUG
	check_tree(bfallctr, 0);
#endif
    }
    else {
	tree_delete(allctr, block);
    }
    DESTROY_LIST_ELEM(x);
}
static Block_t *
bf_get_free_block(Allctr_t *allctr, Uint size,
		  Block_t *cand_blk, Uint cand_size)
{
    BFAllctr_t *bfallctr = (BFAllctr_t *) allctr;
    RBTree_t **root = &bfallctr->mbc_root;
    RBTree_t *x = *root;
    RBTree_t *blk = NULL;
    Uint blk_sz;
    ASSERT(!cand_blk || cand_size >= size);
    while (x) {
	blk_sz = BF_BLK_SZ(x);
	if (blk_sz < size) {
	    x = x->right;
	}
	else {
	    blk = x;
	    if (blk_sz == size)
		break;
	    x = x->left;
	}
    }
    if (!blk)
	return NULL;
    ASSERT(IS_TREE_NODE(blk));
#ifdef HARD_DEBUG
    {
	RBTree_t *ct_blk = check_tree(root, 0, size);
	ASSERT(BF_BLK_SZ(ct_blk) == BF_BLK_SZ(blk));
    }
#endif
    if (cand_blk && cand_size <= BF_BLK_SZ(blk))
	return NULL;
    blk = BF_LIST_NEXT(blk) ? BF_LIST_NEXT(blk) : blk;
    bf_unlink_free_block(allctr, (Block_t *) blk);
    return (Block_t *) blk;
}
static struct {
    Eterm as;
    Eterm aobf;
    Eterm bf;
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
    AM_INIT(aobf);
    AM_INIT(bf);
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
    BFAllctr_t *bfallctr = (BFAllctr_t *) allctr;
    Eterm res = THE_NON_VALUE;
    if (print_to_p) {
	erts_print(*print_to_p,
		   print_to_arg,
		   "%sas: %s\n",
		   prefix,
		   bfallctr->address_order ? "aobf" : "bf");
    }
    if (hpp || szp) {
	if (!atoms_initialized)
	    erts_exit(ERTS_ERROR_EXIT, "%s:%d: Internal error: Atoms not initialized",
		     __FILE__, __LINE__);;
	res = NIL;
	add_2tup(hpp, szp, &res,
		 am.as,
		 bfallctr->address_order ? am.aobf : am.bf);
    }
    return res;
}
UWord
erts_bfalc_test(UWord op, UWord a1, UWord a2)
{
    switch (op) {
    case 0x200:	return (UWord) ((BFAllctr_t *) a1)->address_order;
    case 0x201:	return (UWord) ((BFAllctr_t *) a1)->mbc_root;
    case 0x202:	return (UWord) ((RBTree_t *) a1)->parent;
    case 0x203:	return (UWord) ((RBTree_t *) a1)->left;
    case 0x204:	return (UWord) ((RBTree_t *) a1)->right;
    case 0x205:	return (UWord) BF_LIST_NEXT(a1);
    case 0x206:	return (UWord) IS_BLACK((RBTree_t *) a1);
    case 0x207:	return (UWord) IS_TREE_NODE((RBTree_t *) a1);
    case 0x208:	return (UWord) 1;
    case 0x20a: return (UWord) !((BFAllctr_t *) a1)->address_order;
    case 0x20b:	return (UWord) BF_LIST_PREV(a1);
    default:	ASSERT(0); return ~((UWord) 0);
    }
}
#ifdef HARD_DEBUG
#define IS_LEFT_VISITED(FB)	((FB)->flags & LEFT_VISITED_FLG)
#define IS_RIGHT_VISITED(FB)	((FB)->flags & RIGHT_VISITED_FLG)
#define SET_LEFT_VISITED(FB)	((FB)->flags |= LEFT_VISITED_FLG)
#define SET_RIGHT_VISITED(FB)	((FB)->flags |= RIGHT_VISITED_FLG)
#define UNSET_LEFT_VISITED(FB)	((FB)->flags &= ~LEFT_VISITED_FLG)
#define UNSET_RIGHT_VISITED(FB)	((FB)->flags &= ~RIGHT_VISITED_FLG)
#if 0
#  define PRINT_TREE
#else
#  undef PRINT_TREE
#endif
#ifdef PRINT_TREE
static void print_tree(RBTree_t *, int);
#endif
static RBTree_t *
check_tree(RBTree_t *root, int ao, Uint size)
{
    RBTree_t *res = NULL;
    Sint blacks;
    Sint curr_blacks;
    RBTree_t *x;
#ifdef PRINT_TREE
    print_tree(root, ao);
#endif
    if (!root)
	return res;
    x = root;
    ASSERT(IS_BLACK(x));
    ASSERT(!x->parent);
    curr_blacks = 1;
    blacks = -1;
    while (x) {
	if (!IS_LEFT_VISITED(x)) {
	    SET_LEFT_VISITED(x);
	    if (x->left) {
		x = x->left;
		if (IS_BLACK(x))
		    curr_blacks++;
		continue;
	    }
	    else {
		if (blacks < 0)
		    blacks = curr_blacks;
		ASSERT(blacks == curr_blacks);
	    }
	}
	if (!IS_RIGHT_VISITED(x)) {
	    SET_RIGHT_VISITED(x);
	    if (x->right) {
		x = x->right;
		if (IS_BLACK(x))
		    curr_blacks++;
		continue;
	    }
	    else {
		if (blacks < 0)
		    blacks = curr_blacks;
		ASSERT(blacks == curr_blacks);
	    }
	}
	if (IS_RED(x)) {
	    ASSERT(IS_BLACK(x->right));
	    ASSERT(IS_BLACK(x->left));
	}
	ASSERT(x->parent || x == root);
	if (x->left) {
	    ASSERT(x->left->parent == x);
	    if (ao) {
		ASSERT(BF_BLK_SZ(x->left) < BF_BLK_SZ(x)
		       || (BF_BLK_SZ(x->left) == BF_BLK_SZ(x) && x->left < x));
	    }
	    else {
		ASSERT(IS_TREE_NODE(x->left));
		ASSERT(BF_BLK_SZ(x->left) < BF_BLK_SZ(x));
	    }
	}
	if (x->right) {
	    ASSERT(x->right->parent == x);
	    if (ao) {
		ASSERT(BF_BLK_SZ(x->right) > BF_BLK_SZ(x)
		       || (BF_BLK_SZ(x->right) == BF_BLK_SZ(x) && x->right > x));
	    }
	    else {
		ASSERT(IS_TREE_NODE(x->right));
		ASSERT(BF_BLK_SZ(x->right) > BF_BLK_SZ(x));
	    }
	}
	if (size && BF_BLK_SZ(x) >= size) {
	    if (ao) {
		if (!res
		    || BF_BLK_SZ(x) < BF_BLK_SZ(res)
		    || (BF_BLK_SZ(x) == BF_BLK_SZ(res) && x < res))
		    res = x;
	    }
	    else {
		if (!res || BF_BLK_SZ(x) < BF_BLK_SZ(res))
		    res = x;
	    }
	}
	UNSET_LEFT_VISITED(x);
	UNSET_RIGHT_VISITED(x);
	if (IS_BLACK(x))
	    curr_blacks--;
	x = x->parent;
    }
    ASSERT(curr_blacks == 0);
    UNSET_LEFT_VISITED(root);
    UNSET_RIGHT_VISITED(root);
    return res;
}
#ifdef PRINT_TREE
#define INDENT_STEP 2
#include <stdio.h>
static void
print_tree_aux(RBTree_t *x, int indent)
{
    int i;
    if (!x) {
	for (i = 0; i < indent; i++) {
	    putc(' ', stderr);
	}
	fprintf(stderr, "BLACK: nil\r\n");
    }
    else {
	print_tree_aux(x->right, indent + INDENT_STEP);
	for (i = 0; i < indent; i++) {
	    putc(' ', stderr);
	}
	fprintf(stderr, "%s: sz=%lu addr=0x%lx\r\n",
		IS_BLACK(x) ? "BLACK" : "RED",
		BF_BLK_SZ(x),
		(Uint) x);
	print_tree_aux(x->left,  indent + INDENT_STEP);
    }
}
static void
print_tree(RBTree_t *root, int ao)
{
    char *type = ao ? "Size-Adress" : "Size";
    fprintf(stderr, " --- %s tree begin ---\r\n", type);
    print_tree_aux(root, 0);
    fprintf(stderr, " --- %s tree end ---\r\n", type);
}
#endif
#endif