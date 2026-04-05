#ifdef ERTS_RBT_CMP_KEYS
#  undef ERTS_RBT_IS_LT
#  define ERTS_RBT_IS_LT(KX, KY) (ERTS_RBT_CMP_KEYS((KX), (KY)) < 0)
#  undef ERTS_RBT_IS_EQ
#  define ERTS_RBT_IS_EQ(KX, KY) (ERTS_RBT_CMP_KEYS((KX), (KY)) == 0)
#endif
#ifndef ERTS_RBT_PREFIX
#  error Missing definition of ERTS_RBT_PREFIX
#endif
#ifndef ERTS_RBT_T
#  error Missing definition of ERTS_RBT_T
#endif
#ifndef ERTS_RBT_KEY_T
#  error Missing definition of ERTS_RBT_KEY_T
#endif
#ifndef ERTS_RBT_FLAGS_T
#  error Missing definition of ERTS_RBT_FLAGS_T
#endif
#ifndef ERTS_RBT_INIT_EMPTY_TNODE
#  error Missing definition of ERTS_RBT_INIT_EMPTY_TNODE
#endif
#ifndef ERTS_RBT_IS_RED
#  error Missing definition of ERTS_RBT_IS_RED
#endif
#ifndef ERTS_RBT_SET_RED
#  error Missing definition of ERTS_RBT_SET_RED
#endif
#ifndef ERTS_RBT_IS_BLACK
#  error Missing definition of ERTS_RBT_IS_BLACK
#endif
#ifndef ERTS_RBT_SET_BLACK
#  error Missing definition of ERTS_RBT_SET_BLACK
#endif
#ifndef ERTS_RBT_GET_FLAGS
#  error Missing definition of ERTS_RBT_GET_FLAGS
#endif
#ifndef ERTS_RBT_SET_FLAGS
#  error Missing definition of ERTS_RBT_SET_FLAGS
#endif
#ifndef ERTS_RBT_GET_PARENT
#  error Missing definition of ERTS_RBT_GET_PARENT
#endif
#ifndef ERTS_RBT_SET_PARENT
#  error Missing definition of ERTS_RBT_SET_PARENT
#endif
#ifndef ERTS_RBT_GET_RIGHT
#  error Missing definition of ERTS_RBT_GET_RIGHT
#endif
#ifndef ERTS_RBT_GET_LEFT
#  error Missing definition of ERTS_RBT_GET_LEFT
#endif
#ifndef ERTS_RBT_IS_LT
#  error Missing definition of ERTS_RBT_IS_LT
#endif
#ifndef ERTS_RBT_GET_KEY
#  error Missing definition of ERTS_RBT_GET_KEY
#endif
#ifndef ERTS_RBT_IS_EQ
#  error Missing definition of ERTS_RBT_IS_EQ
#endif
#undef ERTS_RBT_IS_GT__
#ifdef ERTS_RBT_CMP_KEYS
#  define ERTS_RBT_IS_GT__(KX, KY) \
    (ERTS_RBT_CMP_KEYS((KX), (KY)) > 0)
#else
#  define ERTS_RBT_IS_GT__(KX, KY) \
    (!ERTS_RBT_IS_LT((KX), (KY)) && !ERTS_RBT_IS_EQ((KX), (KY)))
#endif
#if defined(ERTS_RBT_HARD_DEBUG) || defined(DEBUG)
#  ifndef ERTS_RBT_DEBUG
#    define ERTS_RBT_DEBUG 1
#  endif
#endif
#if defined(ERTS_RBT_HARD_DEBUG) && defined(__GNUC__)
#warning "* * * * * * * * * * * * * * * * * *"
#warning "* ERTS_RBT_HARD_DEBUG IS ENABLED! *"
#warning "* * * * * * * * * * * * * * * * * *"
#endif
#undef ERTS_RBT_ASSERT
#if defined(ERTS_RBT_DEBUG)
#define ERTS_RBT_ASSERT(E) ERTS_ASSERT(E)
#else
#define ERTS_RBT_ASSERT(E) ((void) 1)
#endif
#undef ERTS_RBT_API_INLINE__
#if defined(ERTS_RBT_NO_API_INLINE) || defined(ERTS_RBT_DEBUG)
#  define ERTS_RBT_API_INLINE__
#else
#  define ERTS_RBT_API_INLINE__ ERTS_INLINE
#endif
#define ERTS_RBT_CONCAT_MACRO_VALUES___(X, Y) \
    X ## Y
#define ERTS_RBT_CONCAT_MACRO_VALUES__(X, Y) \
    ERTS_RBT_CONCAT_MACRO_VALUES___(X, Y)
#undef ERTS_RBT_YIELD_STATE_T__
#define ERTS_RBT_YIELD_STATE_T__ \
    ERTS_RBT_CONCAT_MACRO_VALUES__(ERTS_RBT_PREFIX, _rbt_yield_state_t)
typedef struct {
    ERTS_RBT_T *x;
    int up;
#ifdef DEBUG
    int debug_red_adj;
#endif
} ERTS_RBT_YIELD_STATE_T__;
#define ERTS_RBT_CALLBACK_FOREACH_FUNC(NAME) int (*NAME)(ERTS_RBT_T *, void *, Sint)
#ifndef ERTS_RBT_YIELD_STAT_INITER
#  ifdef DEBUG
#    define ERTS_RBT_YIELD_STAT_INITER {NULL, 0, CONTEXT_REDS}
#  else
#    define ERTS_RBT_YIELD_STAT_INITER {NULL, 0}
#  endif
#endif
#ifndef ERTS_RBT_YIELD_STAT_INIT
#  define ERTS_RBT_YIELD_STAT_INIT__(YS)                                \
    do {                                                                \
        (YS)->x = NULL;                                                 \
        (YS)->up = 0;                                                   \
    } while (0)
#  ifdef DEBUG
#    define ERTS_RBT_YIELD_STAT_INIT(YS)        \
    do {                                        \
        ERTS_RBT_YIELD_STAT_INIT__(YS);         \
        (YS)->debug_red_adj = CONTEXT_REDS;     \
    } while(0)
#  else
#    define ERTS_RBT_YIELD_STAT_INIT(YS) ERTS_RBT_YIELD_STAT_INIT__(YS)
#  endif
#endif
#define ERTS_RBT_FUNC__(Name) \
    ERTS_RBT_CONCAT_MACRO_VALUES__(ERTS_RBT_PREFIX, _rbt_ ## Name)
#undef ERTS_RBT_NEED_REPLACE__
#undef ERTS_RBT_NEED_INSERT__
#undef ERTS_RBT_NEED_ROTATE__
#undef ERTS_RBT_NEED_FOREACH_UNORDERED__
#undef ERTS_RBT_NEED_FOREACH_ORDERED__
#undef ERTS_RBT_NEED_HDBG_CHECK_TREE__
#undef ERTS_RBT_HDBG_CHECK_TREE__
#if defined(ERTS_RBT_WANT_REPLACE) || defined(ERTS_RBT_WANT_DELETE)
#  define ERTS_RBT_NEED_REPLACE__
#endif
#if defined(ERTS_RBT_WANT_INSERT) || defined(ERTS_RBT_WANT_LOOKUP_INSERT)
#  define ERTS_RBT_NEED_INSERT__
#endif
#if defined(ERTS_RBT_WANT_DELETE) || defined(ERTS_RBT_NEED_INSERT__)
#  define ERTS_RBT_NEED_ROTATE__
#endif
#if defined(ERTS_RBT_WANT_FOREACH) \
    || defined(ERTS_RBT_WANT_FOREACH_YIELDING) \
    || defined(ERTS_RBT_WANT_FOREACH_DESTROY) \
    || defined(ERTS_RBT_WANT_FOREACH_DESTROY_YIELDING)
#  define ERTS_RBT_NEED_FOREACH_UNORDERED__
#endif
#if defined(ERTS_RBT_WANT_FOREACH_SMALL) \
    || defined(ERTS_RBT_WANT_FOREACH_LARGE) \
    || defined(ERTS_RBT_WANT_FOREACH_SMALL_YIELDING) \
    || defined(ERTS_RBT_WANT_FOREACH_LARGE_YIELDING) \
    || defined(ERTS_RBT_WANT_FOREACH_SMALL_DESTROY) \
    || defined(ERTS_RBT_WANT_FOREACH_LARGE_DESTROY) \
    || defined(ERTS_RBT_WANT_FOREACH_SMALL_DESTROY_YIELDING) \
    || defined(ERTS_RBT_WANT_FOREACH_LARGE_DESTROY_YIELDING)
#  define ERTS_RBT_NEED_FOREACH_ORDERED__
#endif
#if defined(ERTS_RBT_HARD_DEBUG) \
    && (defined(ERTS_RBT_WANT_DELETE) \
	|| defined(ERTS_RBT_NEED_INSERT__))
static void ERTS_RBT_FUNC__(hdbg_check_tree)(ERTS_RBT_T *root, ERTS_RBT_T *node);
#  define ERTS_RBT_NEED_HDBG_CHECK_TREE__
#  define ERTS_RBT_HDBG_CHECK_TREE__(R,N) \
    ERTS_RBT_FUNC__(hdbg_check_tree)((R),(N))
#else
#  define ERTS_RBT_HDBG_CHECK_TREE__(R,N) ((void) 1)
#endif
#ifdef ERTS_RBT_NEED_ROTATE__
static ERTS_INLINE void
ERTS_RBT_FUNC__(left_rotate__)(ERTS_RBT_T **root, ERTS_RBT_T *x)
{
    ERTS_RBT_T *y, *l, *p;
    y = ERTS_RBT_GET_RIGHT(x);
    l = ERTS_RBT_GET_LEFT(y);
    ERTS_RBT_SET_RIGHT(x, l);
    if (l)
	ERTS_RBT_SET_PARENT(l, x);
    p = ERTS_RBT_GET_PARENT(x);
    ERTS_RBT_SET_PARENT(y, p);
    if (!p) {
	ERTS_RBT_ASSERT(*root == x);
	*root = y;
#ifdef ERTS_RBT_UPDATE_ATTACHED_DATA_CHGROOT
	ERTS_RBT_UPDATE_ATTACHED_DATA_CHGROOT(x, y);
#endif
    }
    else if (x == ERTS_RBT_GET_LEFT(p))
	ERTS_RBT_SET_LEFT(p, y);
    else {
	ERTS_RBT_ASSERT(x == ERTS_RBT_GET_RIGHT(p));
	ERTS_RBT_SET_RIGHT(p, y);
    }
    ERTS_RBT_SET_LEFT(y, x);
    ERTS_RBT_SET_PARENT(x, y);
#ifdef ERTS_RBT_UPDATE_ATTACHED_DATA_ROTATE
    ERTS_RBT_UPDATE_ATTACHED_DATA_ROTATE(!0, x, y);
#endif
}
static ERTS_INLINE void
ERTS_RBT_FUNC__(right_rotate__)(ERTS_RBT_T **root, ERTS_RBT_T *x)
{
    ERTS_RBT_T *y, *r, *p;
    y = ERTS_RBT_GET_LEFT(x);
    r = ERTS_RBT_GET_RIGHT(y);
    ERTS_RBT_SET_LEFT(x, r);
    if (r)
	ERTS_RBT_SET_PARENT(r, x);
    p = ERTS_RBT_GET_PARENT(x);
    ERTS_RBT_SET_PARENT(y, p);
    if (!p) {
	ERTS_RBT_ASSERT(*root == x);
	*root = y;
#ifdef ERTS_RBT_UPDATE_ATTACHED_DATA_CHGROOT
	ERTS_RBT_UPDATE_ATTACHED_DATA_CHGROOT(x, y);
#endif
    }
    else if (x == ERTS_RBT_GET_RIGHT(p))
	ERTS_RBT_SET_RIGHT(p, y);
    else {
	ERTS_RBT_ASSERT(x == ERTS_RBT_GET_LEFT(p));
	ERTS_RBT_SET_LEFT(p, y);
    }
    ERTS_RBT_SET_RIGHT(y, x);
    ERTS_RBT_SET_PARENT(x, y);
#ifdef ERTS_RBT_UPDATE_ATTACHED_DATA_ROTATE
    ERTS_RBT_UPDATE_ATTACHED_DATA_ROTATE(0, x, y);
#endif
}
#endif
#ifdef ERTS_RBT_NEED_REPLACE__
static ERTS_INLINE void
ERTS_RBT_FUNC__(replace__)(ERTS_RBT_T **root, ERTS_RBT_T *x, ERTS_RBT_T *y)
{
    ERTS_RBT_T *p, *r, *l;
    ERTS_RBT_FLAGS_T f;
    p = ERTS_RBT_GET_PARENT(x);
    if (!p) {
	ERTS_RBT_ASSERT(*root == x);
	*root = y;
#ifdef ERTS_RBT_UPDATE_ATTACHED_DATA_CHGROOT
	ERTS_RBT_UPDATE_ATTACHED_DATA_CHGROOT(x, y);
#endif
    }
    else if (x == ERTS_RBT_GET_LEFT(p))
	ERTS_RBT_SET_LEFT(p, y);
    else {
	ERTS_RBT_ASSERT(x == ERTS_RBT_GET_RIGHT(p));
	ERTS_RBT_SET_RIGHT(p, y);
    }
    l = ERTS_RBT_GET_LEFT(x);
    if (l) {
	ERTS_RBT_ASSERT(ERTS_RBT_GET_PARENT(l) == x);
	ERTS_RBT_SET_PARENT(l, y);
    }
    r = ERTS_RBT_GET_RIGHT(x);
    if (r) {
	ERTS_RBT_ASSERT(ERTS_RBT_GET_PARENT(r) == x);
	ERTS_RBT_SET_PARENT(r, y);
    }
    f = ERTS_RBT_GET_FLAGS(x);
    ERTS_RBT_SET_FLAGS(y, f);
    ERTS_RBT_SET_PARENT(y, p);
    ERTS_RBT_SET_RIGHT(y, r);
    ERTS_RBT_SET_LEFT(y, l);
}
#endif
#ifdef ERTS_RBT_WANT_REPLACE
static ERTS_RBT_API_INLINE__ void
ERTS_RBT_FUNC__(replace)(ERTS_RBT_T **root, ERTS_RBT_T *x, ERTS_RBT_T *y)
{
    ERTS_RBT_ASSERT(ERTS_RBT_IS_EQ(ERTS_RBT_GET_KEY(x),
				   ERTS_RBT_GET_KEY(y)));
    ERTS_RBT_FUNC__(replace__)(root, x, y);
}
#endif
#ifdef ERTS_RBT_WANT_DELETE
static ERTS_RBT_API_INLINE__ void
ERTS_RBT_FUNC__(delete)(ERTS_RBT_T **root, ERTS_RBT_T *n)
{
    int spliced_is_black;
    ERTS_RBT_T *p, *x, *y, *z = n;
    ERTS_RBT_T null_x;
    ERTS_RBT_HDBG_CHECK_TREE__(*root, n);
    ERTS_RBT_INIT_EMPTY_TNODE(&null_x);
    if (!ERTS_RBT_GET_LEFT(z) || !ERTS_RBT_GET_RIGHT(z))
	y = z;
    else {
	y = ERTS_RBT_GET_RIGHT(z);
	while (1) {
	    ERTS_RBT_T *t = ERTS_RBT_GET_LEFT(y);
	    if (!t)
		break;
	    y = t;
	}
    }
    x = ERTS_RBT_GET_LEFT(y);
    if (!x)
	x = ERTS_RBT_GET_RIGHT(y);
    spliced_is_black = ERTS_RBT_IS_BLACK(y);
    p = ERTS_RBT_GET_PARENT(y);
    if (x)
	ERTS_RBT_SET_PARENT(x, p);
    else if (spliced_is_black) {
	x = &null_x;
	ERTS_RBT_SET_BLACK(x);
	ERTS_RBT_SET_PARENT(x, p);
	ERTS_RBT_SET_LEFT(y, x);
    }
    if (!p) {
	ERTS_RBT_ASSERT(*root == y);
	*root = x;
#ifdef ERTS_RBT_UPDATE_ATTACHED_DATA_CHGROOT
	ERTS_RBT_UPDATE_ATTACHED_DATA_CHGROOT(y, x);
#endif
    }
    else {
	if (y == ERTS_RBT_GET_LEFT(p))
	    ERTS_RBT_SET_LEFT(p, x);
	else {
	    ERTS_RBT_ASSERT(y == ERTS_RBT_GET_RIGHT(p));
	    ERTS_RBT_SET_RIGHT(p, x);
	}
#ifdef ERTS_RBT_UPDATE_ATTACHED_DATA_DMOD
	if (p != z)
	    ERTS_RBT_UPDATE_ATTACHED_DATA_DMOD(p, y == z ? NULL : z);
#endif
    }
    if (y != z) {
	ERTS_RBT_FUNC__(replace__)(root, z, y);
#ifdef ERTS_RBT_UPDATE_ATTACHED_DATA_DMOD
	ERTS_RBT_UPDATE_ATTACHED_DATA_DMOD(y, NULL);
#endif
    }
    if (spliced_is_black) {
	p = ERTS_RBT_GET_PARENT(x);
	while (ERTS_RBT_IS_BLACK(x) && p) {
	    ERTS_RBT_T *r, *l;
	    if (x == ERTS_RBT_GET_LEFT(p)) {
		y = ERTS_RBT_GET_RIGHT(p);
		ERTS_RBT_ASSERT(y);
		if (ERTS_RBT_IS_RED(y)) {
		    ERTS_RBT_ASSERT(ERTS_RBT_GET_RIGHT(y));
		    ERTS_RBT_ASSERT(ERTS_RBT_GET_LEFT(y));
		    ERTS_RBT_SET_BLACK(y);
		    ERTS_RBT_ASSERT(ERTS_RBT_IS_BLACK(p));
		    ERTS_RBT_SET_RED(p);
		    ERTS_RBT_FUNC__(left_rotate__)(root, p);
		    p = ERTS_RBT_GET_PARENT(x);
		    y = ERTS_RBT_GET_RIGHT(p);
		}
		ERTS_RBT_ASSERT(y);
		ERTS_RBT_ASSERT(ERTS_RBT_IS_BLACK(y));
		l = ERTS_RBT_GET_LEFT(y);
		r = ERTS_RBT_GET_RIGHT(y);
		if ((!l || ERTS_RBT_IS_BLACK(l))
		    && (!r || ERTS_RBT_IS_BLACK(r))) {
		    ERTS_RBT_SET_RED(y);
		    x = p;
		    p = ERTS_RBT_GET_PARENT(x);
		}
		else {
		    if (!r || ERTS_RBT_IS_BLACK(r)) {
			ERTS_RBT_SET_BLACK(l);
			ERTS_RBT_SET_RED(y);
			ERTS_RBT_FUNC__(right_rotate__)(root, y);
			p = ERTS_RBT_GET_PARENT(x);
			y = ERTS_RBT_GET_RIGHT(p);
		    }
		    ERTS_RBT_ASSERT(y);
		    if (p && ERTS_RBT_IS_RED(p)) {
			ERTS_RBT_SET_BLACK(p);
			ERTS_RBT_SET_RED(y);
		    }
		    ERTS_RBT_ASSERT(ERTS_RBT_GET_RIGHT(y));
		    ERTS_RBT_SET_BLACK(ERTS_RBT_GET_RIGHT(y));
		    ERTS_RBT_FUNC__(left_rotate__)(root, p);
		    x = *root;
		    break;
		}
	    }
	    else {
		ERTS_RBT_ASSERT(x == ERTS_RBT_GET_RIGHT(p));
		y = ERTS_RBT_GET_LEFT(p);
		ERTS_RBT_ASSERT(y);
		if (ERTS_RBT_IS_RED(y)) {
		    ERTS_RBT_ASSERT(ERTS_RBT_GET_RIGHT(y));
		    ERTS_RBT_ASSERT(ERTS_RBT_GET_LEFT(y));
		    ERTS_RBT_SET_BLACK(y);
		    ERTS_RBT_ASSERT(ERTS_RBT_IS_BLACK(p));
		    ERTS_RBT_SET_RED(p);
		    ERTS_RBT_FUNC__(right_rotate__)(root, p);
		    p = ERTS_RBT_GET_PARENT(x);
		    y = ERTS_RBT_GET_LEFT(p);
		}
		ERTS_RBT_ASSERT(y);
		ERTS_RBT_ASSERT(ERTS_RBT_IS_BLACK(y));
		l = ERTS_RBT_GET_LEFT(y);
		r = ERTS_RBT_GET_RIGHT(y);
		if ((!r || ERTS_RBT_IS_BLACK(r))
		    && (!l || ERTS_RBT_IS_BLACK(l))) {
		    ERTS_RBT_SET_RED(y);
		    x = p;
		    p = ERTS_RBT_GET_PARENT(x);
		}
		else {
		    if (!l || ERTS_RBT_IS_BLACK(l)) {
			ERTS_RBT_SET_BLACK(r);
			ERTS_RBT_SET_RED(y);
			ERTS_RBT_FUNC__(left_rotate__)(root, y);
			p = ERTS_RBT_GET_PARENT(x);
			y = ERTS_RBT_GET_LEFT(p);
		    }
		    ERTS_RBT_ASSERT(y);
		    if (p && ERTS_RBT_IS_RED(p)) {
			ERTS_RBT_SET_BLACK(p);
			ERTS_RBT_SET_RED(y);
		    }
		    ERTS_RBT_ASSERT(ERTS_RBT_GET_LEFT(y));
		    ERTS_RBT_SET_BLACK(ERTS_RBT_GET_LEFT(y));
		    ERTS_RBT_FUNC__(right_rotate__)(root, p);
		    x = *root;
		    break;
		}
	    }
	}
	ERTS_RBT_SET_BLACK(x);
	x = &null_x;
	p = ERTS_RBT_GET_PARENT(x);
	if (p) {
	    if (ERTS_RBT_GET_LEFT(p) == x)
		ERTS_RBT_SET_LEFT(p, NULL);
	    else {
		ERTS_RBT_ASSERT(ERTS_RBT_GET_RIGHT(p) == x);
		ERTS_RBT_SET_RIGHT(p, NULL);
	    }
	    ERTS_RBT_ASSERT(!ERTS_RBT_GET_LEFT(x));
	    ERTS_RBT_ASSERT(!ERTS_RBT_GET_RIGHT(x));
	}
	else if (*root == x) {
	    *root = NULL;
#ifdef ERTS_RBT_UPDATE_ATTACHED_DATA_CHGROOT
	    ERTS_RBT_UPDATE_ATTACHED_DATA_CHGROOT(x, NULL);
#endif
	    ERTS_RBT_ASSERT(!ERTS_RBT_GET_LEFT(x));
	    ERTS_RBT_ASSERT(!ERTS_RBT_GET_RIGHT(x));
	}
    }
    ERTS_RBT_HDBG_CHECK_TREE__(*root, NULL);
}
#endif
#ifdef ERTS_RBT_NEED_INSERT__
static void
ERTS_RBT_FUNC__(insert_fixup__)(ERTS_RBT_T **root, ERTS_RBT_T *n)
{
    ERTS_RBT_T *x, *y;
    x = n;
    ERTS_RBT_ASSERT(x != *root && ERTS_RBT_IS_RED(ERTS_RBT_GET_PARENT(x)));
    do {
	ERTS_RBT_T *p, *pp;
	p = ERTS_RBT_GET_PARENT(x);
	pp = ERTS_RBT_GET_PARENT(p);
	ERTS_RBT_ASSERT(p && pp);
	ERTS_RBT_ASSERT(ERTS_RBT_IS_RED(x));
	ERTS_RBT_ASSERT(ERTS_RBT_IS_BLACK(pp));
	if (p == ERTS_RBT_GET_LEFT(pp)) {
	    y = ERTS_RBT_GET_RIGHT(pp);
	    if (y && ERTS_RBT_IS_RED(y)) {
		ERTS_RBT_SET_BLACK(y);
		ERTS_RBT_SET_BLACK(p);
		ERTS_RBT_SET_RED(pp);
		x = pp;
	    }
	    else {
		if (x == ERTS_RBT_GET_RIGHT(p)) {
		    x = p;
		    ERTS_RBT_FUNC__(left_rotate__)(root, x);
		    p = ERTS_RBT_GET_PARENT(x);
		    pp = ERTS_RBT_GET_PARENT(p);
		    ERTS_RBT_ASSERT(p && pp);
		}
		ERTS_RBT_ASSERT(x == ERTS_RBT_GET_LEFT(ERTS_RBT_GET_LEFT(pp)));
		ERTS_RBT_ASSERT(ERTS_RBT_IS_RED(x));
		ERTS_RBT_ASSERT(ERTS_RBT_IS_RED(p));
		ERTS_RBT_ASSERT(ERTS_RBT_IS_BLACK(pp));
		ERTS_RBT_ASSERT(!y || ERTS_RBT_IS_BLACK(y));
		ERTS_RBT_SET_BLACK(p);
		ERTS_RBT_SET_RED(pp);
		ERTS_RBT_FUNC__(right_rotate__)(root, pp);
		ERTS_RBT_ASSERT(ERTS_RBT_GET_LEFT(ERTS_RBT_GET_PARENT(x)) == x);
		ERTS_RBT_ASSERT(ERTS_RBT_IS_RED(x));
		ERTS_RBT_ASSERT(ERTS_RBT_IS_RED(
				    ERTS_RBT_GET_RIGHT(
					ERTS_RBT_GET_PARENT(x))));
		ERTS_RBT_ASSERT(!ERTS_RBT_GET_PARENT(x)
				|| ERTS_RBT_IS_BLACK(ERTS_RBT_GET_PARENT(x)));
		break;
	    }
	}
	else {
	    ERTS_RBT_ASSERT(p == ERTS_RBT_GET_RIGHT(pp));
	    y = ERTS_RBT_GET_LEFT(pp);
	    if (y && ERTS_RBT_IS_RED(y)) {
		ERTS_RBT_SET_BLACK(y);
		ERTS_RBT_SET_BLACK(p);
		ERTS_RBT_SET_RED(pp);
		x = pp;
	    }
	    else {
		if (x == ERTS_RBT_GET_LEFT(p)) {
		    x = p;
		    ERTS_RBT_FUNC__(right_rotate__)(root, x);
		    p = ERTS_RBT_GET_PARENT(x);
		    pp = ERTS_RBT_GET_PARENT(p);
		    ERTS_RBT_ASSERT(p && pp);
		}
		ERTS_RBT_ASSERT(x == ERTS_RBT_GET_RIGHT(ERTS_RBT_GET_RIGHT(pp)));
		ERTS_RBT_ASSERT(ERTS_RBT_IS_RED(x));
		ERTS_RBT_ASSERT(ERTS_RBT_IS_RED(p));
		ERTS_RBT_ASSERT(ERTS_RBT_IS_BLACK(pp));
		ERTS_RBT_ASSERT(!y || ERTS_RBT_IS_BLACK(y));
		ERTS_RBT_SET_BLACK(p);
		ERTS_RBT_SET_RED(pp);
		ERTS_RBT_FUNC__(left_rotate__)(root, pp);
		ERTS_RBT_ASSERT(ERTS_RBT_GET_RIGHT(ERTS_RBT_GET_PARENT(x)) == x);
		ERTS_RBT_ASSERT(ERTS_RBT_IS_RED(x));
		ERTS_RBT_ASSERT(ERTS_RBT_IS_RED(
				    ERTS_RBT_GET_LEFT(
					ERTS_RBT_GET_PARENT(x))));
		ERTS_RBT_ASSERT(!ERTS_RBT_GET_PARENT(x)
				|| ERTS_RBT_IS_BLACK(ERTS_RBT_GET_PARENT(x)));
		break;
	    }
	}
    } while (x != *root && ERTS_RBT_IS_RED(ERTS_RBT_GET_PARENT(x)));
    ERTS_RBT_SET_BLACK(*root);
}
static ERTS_INLINE ERTS_RBT_T *
ERTS_RBT_FUNC__(insert_aux__)(ERTS_RBT_T **root, ERTS_RBT_T *n, int lookup)
{
    ERTS_RBT_KEY_T kn = ERTS_RBT_GET_KEY(n);
    ERTS_RBT_HDBG_CHECK_TREE__(*root, NULL);
    ERTS_RBT_INIT_EMPTY_TNODE(n);
    if (!*root) {
	ERTS_RBT_SET_BLACK(n);
	*root = n;
#ifdef ERTS_RBT_UPDATE_ATTACHED_DATA_CHGROOT
	ERTS_RBT_UPDATE_ATTACHED_DATA_CHGROOT(NULL, n);
#endif
    }
    else {
	ERTS_RBT_T *p, *x = *root;
	while (1) {
	    ERTS_RBT_KEY_T kx = ERTS_RBT_GET_KEY(x);
	    ERTS_RBT_T *c;
            int kres;
#ifdef ERTS_RBT_CMP_KEYS
            int kcmp = ERTS_RBT_CMP_KEYS(kn, kx);
            kres = kcmp == 0;
#else
            kres = ERTS_RBT_IS_EQ(kn, kx);
#endif
	    if (lookup && kres) {
		ERTS_RBT_HDBG_CHECK_TREE__(*root, NULL);
		return x;
	    }
#ifdef ERTS_RBT_CMP_KEYS
            kres = kcmp < 0;
#else
            kres = ERTS_RBT_IS_LT(kn, kx);
#endif
	    if (kres) {
		c = ERTS_RBT_GET_LEFT(x);
		if (!c) {
		    ERTS_RBT_SET_PARENT(n, x);
		    ERTS_RBT_SET_LEFT(x, n);
		    p = x;
		    break;
		}
	    }
	    else {
		c = ERTS_RBT_GET_RIGHT(x);
		if (!c) {
		    ERTS_RBT_SET_PARENT(n, x);
		    ERTS_RBT_SET_RIGHT(x, n);
		    p = x;
		    break;
		}
	    }
	    x = c;
	}
	ERTS_RBT_ASSERT(p);
	ERTS_RBT_SET_RED(n);
	if (ERTS_RBT_IS_RED(p))
	    ERTS_RBT_FUNC__(insert_fixup__)(root, n);
    }
    ERTS_RBT_HDBG_CHECK_TREE__(*root, n);
    return NULL;
}
#endif
#ifdef ERTS_RBT_WANT_LOOKUP_INSERT
static ERTS_RBT_API_INLINE__ ERTS_RBT_T *
ERTS_RBT_FUNC__(lookup_insert)(ERTS_RBT_T **root, ERTS_RBT_T *n)
{
    return ERTS_RBT_FUNC__(insert_aux__)(root, n, !0);
}
#endif
#ifdef ERTS_RBT_WANT_INSERT
static ERTS_RBT_API_INLINE__ void
ERTS_RBT_FUNC__(insert)(ERTS_RBT_T **root, ERTS_RBT_T *n)
{
    (void) ERTS_RBT_FUNC__(insert_aux__)(root, n, 0);
}
#endif
#ifdef ERTS_RBT_WANT_LOOKUP_CREATE
static ERTS_INLINE ERTS_RBT_T *
ERTS_RBT_FUNC__(lookup_create)(ERTS_RBT_T **root,
                               ERTS_RBT_KEY_T kn,
                               ERTS_RBT_T *(*create)(ERTS_RBT_KEY_T, void *),
                               void *arg,
                               int *created)
{
    ERTS_RBT_T *n;
    ERTS_RBT_HDBG_CHECK_TREE__(*root, NULL);
    if (!*root) {
        n = (*create)(kn, arg);
        ERTS_RBT_INIT_EMPTY_TNODE(n);
        ERTS_RBT_ASSERT(ERTS_RBT_IS_EQ(ERTS_RBT_GET_KEY(n), kn));
	ERTS_RBT_SET_BLACK(n);
	*root = n;
        *created = !0;
#ifdef ERTS_RBT_UPDATE_ATTACHED_DATA_CHGROOT
	ERTS_RBT_UPDATE_ATTACHED_DATA_CHGROOT(NULL, n);
#endif
    }
    else {
	ERTS_RBT_T *p, *x = *root;
	while (1) {
	    ERTS_RBT_KEY_T kx = ERTS_RBT_GET_KEY(x);
	    ERTS_RBT_T *c;
            int kres;
#ifdef ERTS_RBT_CMP_KEYS
            int kcmp = ERTS_RBT_CMP_KEYS(kn, kx);
            kres = kcmp == 0;
#else
            kres = ERTS_RBT_IS_EQ(kn, kx);
#endif
	    if (kres) {
		ERTS_RBT_HDBG_CHECK_TREE__(*root, NULL);
                *created = 0;
		return x;
	    }
#ifdef ERTS_RBT_CMP_KEYS
            kres = kcmp < 0;
#else
            kres = ERTS_RBT_IS_LT(kn, kx);
#endif
	    if (kres) {
		c = ERTS_RBT_GET_LEFT(x);
		if (!c) {
                    n = (*create)(kn, arg);
                    ERTS_RBT_INIT_EMPTY_TNODE(n);
                    ERTS_RBT_ASSERT(ERTS_RBT_IS_EQ(ERTS_RBT_GET_KEY(n), kn));
                    *created = !0;
		    ERTS_RBT_SET_PARENT(n, x);
		    ERTS_RBT_SET_LEFT(x, n);
		    p = x;
		    break;
		}
	    }
	    else {
		c = ERTS_RBT_GET_RIGHT(x);
		if (!c) {
                    n = (*create)(kn, arg);
                    ERTS_RBT_INIT_EMPTY_TNODE(n);
                    ERTS_RBT_ASSERT(ERTS_RBT_IS_EQ(ERTS_RBT_GET_KEY(n), kn));
                    *created = !0;
		    ERTS_RBT_SET_PARENT(n, x);
		    ERTS_RBT_SET_RIGHT(x, n);
		    p = x;
		    break;
		}
	    }
	    x = c;
	}
        ERTS_RBT_ASSERT(ERTS_RBT_IS_EQ(ERTS_RBT_GET_KEY(n), kn));
	ERTS_RBT_ASSERT(p);
	ERTS_RBT_SET_RED(n);
	if (ERTS_RBT_IS_RED(p))
	    ERTS_RBT_FUNC__(insert_fixup__)(root, n);
    }
    ERTS_RBT_HDBG_CHECK_TREE__(*root, n);
    return n;
}
#endif
#ifdef ERTS_RBT_WANT_LOOKUP
static ERTS_RBT_API_INLINE__ ERTS_RBT_T *
ERTS_RBT_FUNC__(lookup)(ERTS_RBT_T *root, ERTS_RBT_KEY_T key)
{
    ERTS_RBT_T *x = root;
    if (!x)
	return NULL;
    while (1) {
	ERTS_RBT_KEY_T kx = ERTS_RBT_GET_KEY(x);
	ERTS_RBT_T *c;
        int kres;
#ifdef ERTS_RBT_CMP_KEYS
        int kcmp = ERTS_RBT_CMP_KEYS(key, kx);
        kres = kcmp == 0;
#else
        kres = ERTS_RBT_IS_EQ(key, kx);
#endif
	if (kres)
	    return x;
#ifdef ERTS_RBT_CMP_KEYS
        kres = kcmp < 0;
#else
        kres = ERTS_RBT_IS_LT(key, kx);
#endif
	if (kres) {
	    c = ERTS_RBT_GET_LEFT(x);
	    if (!c)
		return NULL;
	}
	else {
	    c = ERTS_RBT_GET_RIGHT(x);
	    if (!c)
		return NULL;
	}
	x = c;
    }
}
#endif
#ifdef ERTS_RBT_WANT_SMALLEST
static ERTS_RBT_API_INLINE__ ERTS_RBT_T *
ERTS_RBT_FUNC__(smallest)(ERTS_RBT_T *root)
{
    ERTS_RBT_T *x = root;
    if (!x)
	return NULL;
    while (1) {
	ERTS_RBT_T *c = ERTS_RBT_GET_LEFT(x);
	if (!c)
	    break;
	x = c;
    }
    return x;
}
#endif
#ifdef ERTS_RBT_WANT_LARGEST
static ERTS_RBT_API_INLINE__ ERTS_RBT_T *
ERTS_RBT_FUNC__(largest)(ERTS_RBT_T *root)
{
    ERTS_RBT_T *x = root;
    if (!x)
	return NULL;
    while (1) {
	ERTS_RBT_T *c = ERTS_RBT_GET_RIGHT(x);
	if (!c)
	    break;
	x = c;
    }
    return x;
}
#endif
#ifdef ERTS_RBT_NEED_FOREACH_UNORDERED__
static ERTS_INLINE int
ERTS_RBT_FUNC__(foreach_unordered__)(ERTS_RBT_T **root,
				     int destroying,
				     ERTS_RBT_CALLBACK_FOREACH_FUNC(op),
				     void *arg,
                                     int yielding,
				     ERTS_RBT_YIELD_STATE_T__ *ystate,
				     Sint reds)
{
    ERTS_RBT_T *c, *p, *x;
    ERTS_RBT_ASSERT(!yielding || ystate);
    if (yielding && ystate->x) {
	x = ystate->x;
#ifdef DEBUG
        if (ystate->debug_red_adj > 0)
            ystate->debug_red_adj -= 100;
#endif
	ERTS_RBT_ASSERT(ystate->up);
	goto restart_up;
    }
    else {
	x = *root;
	if (!x)
	    return reds;
    }
    while (1) {
	while (1) {
	    while (1) {
		c = ERTS_RBT_GET_LEFT(x);
		if (!c)
		    break;
		x = c;
	    }
	    c = ERTS_RBT_GET_RIGHT(x);
	    if (!c)
		break;
	    x = c;
	}
	while (1) {
#ifdef ERTS_RBT_DEBUG
	    int cdir;
            ERTS_UNDEF(cdir,0);
#endif
	    if (yielding && reds <= 0) {
		ystate->x = x;
		ystate->up = 1;
		return 0;
	    }
	restart_up:
	    p = ERTS_RBT_GET_PARENT(x);
#ifdef ERTS_RBT_DEBUG
	    ERTS_RBT_ASSERT(!destroying || !ERTS_RBT_GET_LEFT(x));
	    ERTS_RBT_ASSERT(!destroying || !ERTS_RBT_GET_RIGHT(x));
	    if (p) {
		if (x == ERTS_RBT_GET_LEFT(p)) {
		    cdir = -1;
		    if (destroying)
			ERTS_RBT_SET_LEFT(p, NULL);
		}
		else {
		    ERTS_RBT_ASSERT(x == ERTS_RBT_GET_RIGHT(p));
		    cdir = 1;
		    if (destroying)
			ERTS_RBT_SET_RIGHT(p, NULL);
		}
	    }
#endif
            reds -= (*op)(x, arg, reds);
#ifdef DEBUG
            if (yielding)
                reds -= ystate->debug_red_adj;
#endif
	    if (!p) {
                if (destroying) {
                    *root = NULL;
                }
		if (yielding) {
		    ystate->x = NULL;
		    ystate->up = 0;
                    return reds <= 0 ? 1 : reds;
		}
                return 1;
	    }
	    c = ERTS_RBT_GET_RIGHT(p);
	    if (c && c != x) {
		ERTS_RBT_ASSERT(cdir < 0);
		x = c;
		break;
	    }
	    x = p;
	}
    }
}
#endif
#ifdef ERTS_RBT_NEED_FOREACH_ORDERED__
static ERTS_INLINE int
ERTS_RBT_FUNC__(foreach_ordered__)(ERTS_RBT_T **root,
				   int from_small,
				   int destroying,
                                   ERTS_RBT_CALLBACK_FOREACH_FUNC(op),
                                   ERTS_RBT_CALLBACK_FOREACH_FUNC(destroy),
				   void *arg,
                                   int yielding,
				   ERTS_RBT_YIELD_STATE_T__ *ystate,
				   Sint reds)
{
    ERTS_RBT_T *c, *p, *x;
    ERTS_RBT_ASSERT(!yielding || ystate);
    ERTS_RBT_ASSERT(!destroying || destroy);
    ERTS_RBT_ASSERT(!yielding || yop);
    ERTS_RBT_ASSERT(yielding || op);
    if (yielding && ystate->x) {
	x = ystate->x;
#ifdef DEBUG
        if (ystate->debug_red_adj > 0)
            ystate->debug_red_adj -= 100;
#endif
	if (ystate->up)
	    goto restart_up;
	else
	    goto restart_down;
    }
    else {
	x = *root;
	if (!x)
	    return reds;
    }
    while (1) {
	while (1) {
	    while (1) {
		c = from_small ? ERTS_RBT_GET_LEFT(x) : ERTS_RBT_GET_RIGHT(x);
		if (!c)
		    break;
		x = c;
	    }
            reds -= (*op)(x, arg, reds);
#ifdef DEBUG
            if (yielding)
                reds -= ystate->debug_red_adj;
#endif
	    if (yielding && reds <= 0) {
		ystate->x = x;
		ystate->up = 0;
		return 0;
	    }
	restart_down:
	    c = from_small ? ERTS_RBT_GET_RIGHT(x) : ERTS_RBT_GET_LEFT(x);
	    if (!c)
		break;
	    x = c;
	}
	while (1) {
	    p = ERTS_RBT_GET_PARENT(x);
	    if (p) {
		c = from_small ? ERTS_RBT_GET_RIGHT(p) : ERTS_RBT_GET_LEFT(p);
		if (!c || c != x) {
		    ERTS_RBT_ASSERT((from_small
				     ? ERTS_RBT_GET_LEFT(p)
				     : ERTS_RBT_GET_RIGHT(p)) == x);
                    reds -= (*op)(p, arg, reds);
#ifdef DEBUG
                    if (yielding)
                        reds -= ystate->debug_red_adj;
#endif
		    if (yielding && reds <= 0) {
			ystate->x = x;
			ystate->up = 1;
			return 0;
		    restart_up:
			p = ERTS_RBT_GET_PARENT(x);
		    }
		}
		if (c && c != x) {
		    ERTS_RBT_ASSERT((from_small
				     ? ERTS_RBT_GET_LEFT(p)
				     : ERTS_RBT_GET_RIGHT(p)) == x);
		    x = c;
		    break;
		}
	    }
	    if (destroying) {
#ifdef ERTS_RBT_DEBUG
		ERTS_RBT_ASSERT(!ERTS_RBT_GET_LEFT(x)
				&& !ERTS_RBT_GET_RIGHT(x));
		if (p) {
		    if (x == ERTS_RBT_GET_LEFT(p))
			ERTS_RBT_SET_LEFT(p, NULL);
		    else {
			ERTS_RBT_ASSERT(x == ERTS_RBT_GET_RIGHT(p));
			ERTS_RBT_SET_RIGHT(p, NULL);
		    }
		}
#endif
		reds -= (*destroy)(x, arg, reds);
#ifdef DEBUG
                if (yielding)
                    reds -= ystate->debug_red_adj;
#endif
	    }
	    if (!p) {
                if (destroying) {
                    *root = NULL;
                }
		if (yielding) {
		    ystate->x = NULL;
		    ystate->up = 0;
                    return reds <= 0 ? 1 : reds;
		}
		return 1;
	    }
	    x = p;
	}
    }
}
#endif
#ifdef ERTS_RBT_WANT_FOREACH
static ERTS_RBT_API_INLINE__ void
ERTS_RBT_FUNC__(foreach)(ERTS_RBT_T *root,
			 ERTS_RBT_CALLBACK_FOREACH_FUNC(op),
			 void *arg)
{
    (void) ERTS_RBT_FUNC__(foreach_unordered__)(&root, 0, op, arg,
						0, NULL, 0);
}
#endif
#ifdef ERTS_RBT_WANT_FOREACH_SMALL
static ERTS_RBT_API_INLINE__ void
ERTS_RBT_FUNC__(foreach_small)(ERTS_RBT_T *root,
			       ERTS_RBT_CALLBACK_FOREACH_FUNC(op),
			       void *arg)
{
    (void) ERTS_RBT_FUNC__(foreach_ordered__)(&root, 1, 0,
					      op, NULL, arg,
					      0, NULL, 0);
}
#endif
#ifdef ERTS_RBT_WANT_FOREACH_LARGE
static ERTS_RBT_API_INLINE__ void
ERTS_RBT_FUNC__(foreach_large)(ERTS_RBT_T *root,
			       ERTS_RBT_CALLBACK_FOREACH_FUNC(op),
			       void *arg)
{
    (void) ERTS_RBT_FUNC__(foreach_ordered__)(&root, 0, 0,
					      op, NULL, arg,
					      0, NULL, 0);
}
#endif
#ifdef ERTS_RBT_WANT_FOREACH_YIELDING
static ERTS_RBT_API_INLINE__ int
ERTS_RBT_FUNC__(foreach_yielding)(ERTS_RBT_T *root,
				  ERTS_RBT_CALLBACK_FOREACH_FUNC(op),
				  void *arg,
				  ERTS_RBT_YIELD_STATE_T__ *ystate,
				  Sint reds)
{
    return ERTS_RBT_FUNC__(foreach_unordered__)(&root, 0, op, arg,
						1, ystate, reds);
}
#endif
#ifdef ERTS_RBT_WANT_FOREACH_SMALL_YIELDING
static ERTS_RBT_API_INLINE__ int
ERTS_RBT_FUNC__(foreach_small_yielding)(ERTS_RBT_T *root,
					ERTS_RBT_CALLBACK_FOREACH_FUNC(op),
					void *arg,
					ERTS_RBT_YIELD_STATE_T__ *ystate,
					Sint reds)
{
    return ERTS_RBT_FUNC__(foreach_ordered__)(&root, 1, 0,
					      op, NULL, arg,
					      1, ystate, reds);
}
#endif
#ifdef ERTS_RBT_WANT_FOREACH_LARGE_YIELDING
static ERTS_RBT_API_INLINE__ int
ERTS_RBT_FUNC__(foreach_large_yielding)(ERTS_RBT_T *root,
					ERTS_RBT_CALLBACK_FOREACH_FUNC(op),
					void *arg,
					ERTS_RBT_YIELD_STATE_T__ *ystate,
					Sint reds)
{
    return ERTS_RBT_FUNC__(foreach_ordered__)(&root, 0, 0,
					      op, NULL, arg,
					      1, ystate, reds);
}
#endif
#ifdef ERTS_RBT_WANT_FOREACH_DESTROY
static ERTS_RBT_API_INLINE__ void
ERTS_RBT_FUNC__(foreach_destroy)(ERTS_RBT_T **root,
				 ERTS_RBT_CALLBACK_FOREACH_FUNC(op),
				 void *arg)
{
    (void) ERTS_RBT_FUNC__(foreach_unordered__)(root, 1, op, arg,
                                                0, NULL, 0);
}
#endif
#ifdef ERTS_RBT_WANT_FOREACH_SMALL_DESTROY
static ERTS_RBT_API_INLINE__ void
ERTS_RBT_FUNC__(foreach_small_destroy)(ERTS_RBT_T **root,
				       ERTS_RBT_CALLBACK_FOREACH_FUNC(op),
				       ERTS_RBT_CALLBACK_FOREACH_FUNC(destr),
				       void *arg)
{
    (void) ERTS_RBT_FUNC__(foreach_ordered__)(root, 1, 1,
					      op, destr, arg,
					      0, NULL, 0);
}
#endif
#ifdef ERTS_RBT_WANT_FOREACH_LARGE_DESTROY
static ERTS_RBT_API_INLINE__ void
ERTS_RBT_FUNC__(foreach_large_destroy)(ERTS_RBT_T **root,
				       ERTS_RBT_CALLBACK_FOREACH_FUNC(op),
				       ERTS_RBT_CALLBACK_FOREACH_FUNC(destr),
				       void *arg)
{
    (void) ERTS_RBT_FUNC__(foreach_ordered__)(root, 0, 1,
					      op, destr, arg,
					      0, NULL, 0);
}
#endif
#ifdef ERTS_RBT_WANT_FOREACH_DESTROY_YIELDING
static ERTS_RBT_API_INLINE__ int
ERTS_RBT_FUNC__(foreach_destroy_yielding)(ERTS_RBT_T **root,
					  ERTS_RBT_CALLBACK_FOREACH_FUNC(op),
					  void *arg,
					  ERTS_RBT_YIELD_STATE_T__ *ystate,
					  Sint reds)
{
    return ERTS_RBT_FUNC__(foreach_unordered__)(root, 1, op, arg,
						1, ystate, reds);
}
#endif
#ifdef ERTS_RBT_WANT_FOREACH_SMALL_DESTROY_YIELDING
static ERTS_RBT_API_INLINE__ int
ERTS_RBT_FUNC__(foreach_small_destroy_yielding)(ERTS_RBT_T **root,
						ERTS_RBT_CALLBACK_FOREACH_FUNC(op),
						ERTS_RBT_CALLBACK_FOREACH_FUNC(destr),
						void *arg,
						ERTS_RBT_YIELD_STATE_T__ *ystate,
						Sint reds)
{
    return ERTS_RBT_FUNC__(foreach_ordered__)(root, 1, 1,
					      op, destr, arg,
					      1, ystate, reds);
}
#endif
#ifdef ERTS_RBT_WANT_FOREACH_LARGE_DESTROY_YIELDING
static ERTS_RBT_API_INLINE__ int
ERTS_RBT_FUNC__(foreach_large_destroy_yielding)(ERTS_RBT_T **root,
						ERTS_RBT_CALLBACK_FOREACH_FUNC(op),
						ERTS_RBT_CALLBACK_FOREACH_FUNC(destr),
						void *arg,
						ERTS_RBT_YIELD_STATE_T__ *ystate,
						Sint reds)
{
    return ERTS_RBT_FUNC__(foreach_ordered__)(root, 0, 1,
					      op, destr, arg,
					      1, ystate, reds);
}
#endif
#ifdef ERTS_RBT_WANT_DEBUG_PRINT
static void
ERTS_RBT_FUNC__(debug_print)(FILE *filep, ERTS_RBT_T *x, int indent,
			     void (*print_node)(ERTS_RBT_T *))
{
    if (x) {
	ERTS_RBT_FUNC__(debug_print)(filep, ERTS_RBT_GET_RIGHT(x),
				     indent+2, print_node);
	erts_fprintf(filep,
		     "%*s[%s:%p:",
		     indent, "",
		     ERTS_RBT_IS_BLACK(x) ? "Black" : "Red",
		     x);
	(*print_node)(x);
	erts_fprintf(filep, "]\n");
	ERTS_RBT_FUNC__(debug_print)(filep, ERTS_RBT_GET_LEFT(x),
				     indent+2, print_node);
    }
}
#endif
#ifdef ERTS_RBT_NEED_HDBG_CHECK_TREE__
static void
ERTS_RBT_FUNC__(hdbg_check_tree)(ERTS_RBT_T *root, ERTS_RBT_T *n)
{
    int black_depth = -1, no_black = 0;
    ERTS_RBT_T *c, *p, *x = root;
    ERTS_RBT_KEY_T kx;
    ERTS_RBT_KEY_T kc;
    if (!x) {
        ERTS_RBT_ASSERT(!n);
	return;
    }
    ERTS_RBT_ASSERT(!ERTS_RBT_GET_PARENT(x));
    while (1) {
	while (1) {
	    while (1) {
                if (x == n)
                    n = NULL;
		if (ERTS_RBT_IS_BLACK(x))
		    no_black++;
		else {
		    c = ERTS_RBT_GET_RIGHT(x);
		    ERTS_RBT_ASSERT(!c || ERTS_RBT_IS_BLACK(c));
		    c = ERTS_RBT_GET_LEFT(x);
		    ERTS_RBT_ASSERT(!c || ERTS_RBT_IS_BLACK(c));
		}
		c = ERTS_RBT_GET_LEFT(x);
		if (!c)
		    break;
		ERTS_RBT_ASSERT(x == ERTS_RBT_GET_PARENT(c));
		kx = ERTS_RBT_GET_KEY(x);
		kc = ERTS_RBT_GET_KEY(c);
                ERTS_RBT_ASSERT(!ERTS_RBT_IS_GT__(kc, kx));
		x = c;
	    }
	    c = ERTS_RBT_GET_RIGHT(x);
	    if (!c) {
		if (black_depth < 0)
		    black_depth = no_black;
		ERTS_RBT_ASSERT(black_depth == no_black);
		break;
	    }
	    ERTS_RBT_ASSERT(x == ERTS_RBT_GET_PARENT(c));
	    kx = ERTS_RBT_GET_KEY(x);
	    kc = ERTS_RBT_GET_KEY(c);
            ERTS_RBT_ASSERT(!ERTS_RBT_IS_GT__(kx, kc));
	    x = c;
	}
	while (1) {
	    p = ERTS_RBT_GET_PARENT(x);
	    if (ERTS_RBT_IS_BLACK(x))
		no_black--;
	    if (p) {
		ERTS_RBT_ASSERT(x == ERTS_RBT_GET_LEFT(p)
				|| x == ERTS_RBT_GET_RIGHT(p));
		c = ERTS_RBT_GET_RIGHT(p);
		if (c && c != x) {
		    ERTS_RBT_ASSERT(ERTS_RBT_GET_LEFT(p) == x);
		    kx = ERTS_RBT_GET_KEY(x);
		    kc = ERTS_RBT_GET_KEY(c);
                    ERTS_RBT_ASSERT(!ERTS_RBT_IS_GT__(kx, kc));
		    x = c;
		    break;
		}
	    }
	    if (!p) {
		ERTS_RBT_ASSERT(root == x);
		ERTS_RBT_ASSERT(no_black == 0);
                ERTS_RBT_ASSERT(!n);
		return;
	    }
	    x = p;
	}
    }
}
#undef ERTS_RBT_PRINT_TREE__
#endif
#undef ERTS_RBT_ASSERT
#undef ERTS_RBT_DEBUG
#undef ERTS_RBT_API_INLINE__
#undef ERTS_RBT_YIELD_STATE_T__
#undef ERTS_RBT_NEED_REPLACE__
#undef ERTS_RBT_NEED_INSERT__
#undef ERTS_RBT_NEED_ROTATE__
#undef ERTS_RBT_NEED_FOREACH_UNORDERED__
#undef ERTS_RBT_NEED_FOREACH_ORDERED__
#undef ERTS_RBT_NEED_HDBG_CHECK_TREE__
#undef ERTS_RBT_HDBG_CHECK_TREE__
#undef ERTS_RBT_IS_GT__
#ifdef ERTS_RBT_UNDEF
#  undef ERTS_RBT_PREFIX
#  undef ERTS_RBT_T
#  undef ERTS_RBT_CALLBACK_FOREACH_FUNC
#  undef ERTS_RBT_KEY_T
#  undef ERTS_RBT_FLAGS_T
#  undef ERTS_RBT_INIT_EMPTY_TNODE
#  undef ERTS_RBT_IS_RED
#  undef ERTS_RBT_SET_RED
#  undef ERTS_RBT_IS_BLACK
#  undef ERTS_RBT_SET_BLACK
#  undef ERTS_RBT_GET_FLAGS
#  undef ERTS_RBT_SET_FLAGS
#  undef ERTS_RBT_GET_PARENT
#  undef ERTS_RBT_SET_PARENT
#  undef ERTS_RBT_GET_RIGHT
#  undef ERTS_RBT_SET_RIGHT
#  undef ERTS_RBT_GET_LEFT
#  undef ERTS_RBT_SET_LEFT
#  undef ERTS_RBT_GET_KEY
#  undef ERTS_RBT_CMP_KEYS
#  undef ERTS_RBT_IS_LT
#  undef ERTS_RBT_IS_EQ
#  undef ERTS_RBT_UNDEF
#  undef ERTS_RBT_NO_API_INLINE
#  undef ERTS_RBT_UPDATE_ATTACHED_DATA_ROTATE
#  undef ERTS_RBT_UPDATE_ATTACHED_DATA_DMOD
#  undef ERTS_RBT_UPDATE_ATTACHED_DATA_CHGROOT
#  undef ERTS_RBT_WANT_DELETE
#  undef ERTS_RBT_WANT_INSERT
#  undef ERTS_RBT_WANT_LOOKUP_INSERT
#  undef ERTS_RBT_WANT_REPLACE
#  undef ERTS_RBT_WANT_LOOKUP
#  undef ERTS_RBT_WANT_SMALLEST
#  undef ERTS_RBT_WANT_LARGEST
#  undef ERTS_RBT_WANT_FOREACH
#  undef ERTS_RBT_WANT_FOREACH_DESTROY
#  undef ERTS_RBT_WANT_FOREACH_YIELDING
#  undef ERTS_RBT_WANT_FOREACH_DESTROY_YIELDING
#  undef ERTS_RBT_WANT_FOREACH_SMALL
#  undef ERTS_RBT_WANT_FOREACH_LARGE
#  undef ERTS_RBT_WANT_FOREACH_SMALL_DESTROY
#  undef ERTS_RBT_WANT_FOREACH_LARGE_DESTROY
#  undef ERTS_RBT_WANT_FOREACH_SMALL_YIELDING
#  undef ERTS_RBT_WANT_FOREACH_LARGE_YIELDING
#  undef ERTS_RBT_WANT_FOREACH_SMALL_DESTROY_YIELDING
#  undef ERTS_RBT_WANT_FOREACH_LARGE_DESTROY_YIELDING
#  undef ERTS_RBT_WANT_DEBUG_PRINT
#endif