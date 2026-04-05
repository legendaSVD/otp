#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "sys.h"
#include "beam_load.h"
#include "erl_map.h"
#include "beam_transform_helpers.h"
typedef struct SortBeamOpArg {
    Eterm term;
    BeamOpArg arg;
} SortBeamOpArg;
static int oparg_compare(BeamOpArg* a, BeamOpArg* b);
static int oparg_term_compare(SortBeamOpArg* a, SortBeamOpArg* b);
int
beam_load_safe_mul(UWord a, UWord b, UWord* resp)
{
    Uint res = a * b;
    *resp = res;
    if (b == 0) {
        return 1;
    } else {
        return (res / b) == a;
    }
}
int
beam_load_map_key_sort(LoaderState* stp, BeamOpArg Size, BeamOpArg* Rest)
{
    SortBeamOpArg* t;
    unsigned size = Size.val;
    unsigned i;
    if (size == 2) {
	return 1;
    }
    t = (SortBeamOpArg *) erts_alloc(ERTS_ALC_T_TMP, size*sizeof(SortBeamOpArg));
    for (i = 0; i < size; i += 2) {
	t[i].arg = Rest[i];
	switch (Rest[i].type) {
	case TAG_a:
	    t[i].term = Rest[i].val;
	    ASSERT(is_atom(t[i].term));
	    break;
	case TAG_i:
	    t[i].term = make_small(Rest[i].val);
	    break;
	case TAG_n:
	    t[i].term = NIL;
	    break;
	case TAG_q:
	    t[i].term = beamfile_get_literal(&stp->beam, Rest[i].val);
	    break;
	default:
	    erts_free(ERTS_ALC_T_TMP, (void *) t);
	    return 0;
	}
#ifdef DEBUG
	t[i+1].term = THE_NON_VALUE;
#endif
	t[i+1].arg = Rest[i+1];
    }
    qsort((void *) t, size / 2, 2 * sizeof(SortBeamOpArg),
	  (int (*)(const void *, const void *)) oparg_term_compare);
    for (i = 0; i < size; i++) {
	Rest[i] = t[i].arg;
    }
    erts_free(ERTS_ALC_T_TMP, (void *) t);
    return 1;
}
Eterm
beam_load_get_term(LoaderState* stp, BeamOpArg Key)
{
    switch (Key.type) {
    case TAG_a:
        return Key.val;
    case TAG_i:
        return make_small(Key.val);
    case TAG_n:
        return NIL;
    case TAG_q:
        return beamfile_get_literal(&stp->beam, Key.val);
    default:
        return THE_NON_VALUE;
    }
}
void
beam_load_sort_select_vals(BeamOpArg* base, size_t n)
{
    qsort(base, n, 2 * sizeof(BeamOpArg),
          (int (*)(const void *, const void *)) oparg_compare);
}
static int
oparg_compare(BeamOpArg* a, BeamOpArg* b)
{
    if (a->val < b->val)
        return -1;
    else if (a->val == b->val)
        return 0;
    else
        return 1;
}
static int
oparg_term_compare(SortBeamOpArg* a, SortBeamOpArg* b)
{
    Sint res = erts_cmp_flatmap_keys(a->term, b->term);
    if (res < 0) {
        return -1;
    } else if (res > 0) {
        return 1;
    }
    return 0;
}