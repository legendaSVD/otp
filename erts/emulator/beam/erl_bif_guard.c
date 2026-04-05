#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "sys.h"
#include "erl_vm.h"
#include "global.h"
#include "erl_process.h"
#include "error.h"
#include "bif.h"
#include "big.h"
#include "erl_binary.h"
#include "erl_map.h"
static Eterm double_to_integer(Process* p, double x);
static BIF_RETTYPE erlang_length_trap(BIF_ALIST_3);
static Export erlang_length_export;
void erts_init_bif_guard(void)
{
    erts_init_trap_export(&erlang_length_export,
			  am_erlang, am_length, 3,
			  &erlang_length_trap);
}
BIF_RETTYPE abs_1(BIF_ALIST_1)
{
    Eterm res;
    Sint i0, i;
    Eterm* hp;
    if (is_small(BIF_ARG_1)) {
	i0 = signed_val(BIF_ARG_1);
	i = ERTS_SMALL_ABS(i0);
	if (i0 == MIN_SMALL) {
	    hp = HeapFragOnlyAlloc(BIF_P, BIG_UINT_HEAP_SIZE);
	    BIF_RET(uint_to_big(i, hp));
	} else {
	    BIF_RET(make_small(i));
	}
    } else if (is_big(BIF_ARG_1)) {
	if (!big_sign(BIF_ARG_1)) {
	    BIF_RET(BIF_ARG_1);
	} else {
	    int sz = big_arity(BIF_ARG_1) + 1;
	    Uint* x;
	    hp = HeapFragOnlyAlloc(BIF_P, sz);
	    sz--;
	    res = make_big(hp);
	    x = big_val(BIF_ARG_1);
	    *hp++ = make_pos_bignum_header(sz);
	    x++;
	    while(sz--)
		*hp++ = *x++;
	    BIF_RET(res);
	}
    } else if (is_float(BIF_ARG_1)) {
	FloatDef f;
	GET_DOUBLE(BIF_ARG_1, f);
	if (f.fd <= 0.0) {
	    hp = HeapFragOnlyAlloc(BIF_P, FLOAT_SIZE_OBJECT);
	    f.fd = fabs(f.fd);
	    res = make_float(hp);
	    PUT_DOUBLE(f, hp);
	    BIF_RET(res);
	}
	else
	    BIF_RET(BIF_ARG_1);
    }
    BIF_ERROR(BIF_P, BADARG);
}
BIF_RETTYPE float_1(BIF_ALIST_1)
{
    Eterm res;
    Eterm* hp;
    FloatDef f;
    if (is_not_integer(BIF_ARG_1)) {
	if (is_float(BIF_ARG_1))  {
	    BIF_RET(BIF_ARG_1);
	} else {
	badarg:
	    BIF_ERROR(BIF_P, BADARG);
	}
    }
    if (is_small(BIF_ARG_1)) {
	Sint i = signed_val(BIF_ARG_1);
	f.fd = i;
    } else if (big_to_double(BIF_ARG_1, &f.fd) < 0) {
	goto badarg;
    }
    hp = HeapFragOnlyAlloc(BIF_P, FLOAT_SIZE_OBJECT);
    res = make_float(hp);
    PUT_DOUBLE(f, hp);
    BIF_RET(res);
}
BIF_RETTYPE trunc_1(BIF_ALIST_1)
{
    Eterm res;
    FloatDef f;
    if (is_not_float(BIF_ARG_1)) {
	if (is_integer(BIF_ARG_1))
	    BIF_RET(BIF_ARG_1);
	BIF_ERROR(BIF_P, BADARG);
    }
    GET_DOUBLE(BIF_ARG_1, f);
    res = double_to_integer(BIF_P, (f.fd >= 0.0) ? floor(f.fd) : ceil(f.fd));
    BIF_RET(res);
}
BIF_RETTYPE floor_1(BIF_ALIST_1)
{
    Eterm res;
    FloatDef f;
    if (is_not_float(BIF_ARG_1)) {
	if (is_integer(BIF_ARG_1))
	    BIF_RET(BIF_ARG_1);
	BIF_ERROR(BIF_P, BADARG);
    }
    GET_DOUBLE(BIF_ARG_1, f);
    res = double_to_integer(BIF_P, floor(f.fd));
    BIF_RET(res);
}
BIF_RETTYPE ceil_1(BIF_ALIST_1)
{
    Eterm res;
    FloatDef f;
    if (is_not_float(BIF_ARG_1)) {
	if (is_integer(BIF_ARG_1))
	    BIF_RET(BIF_ARG_1);
	BIF_ERROR(BIF_P, BADARG);
    }
    GET_DOUBLE(BIF_ARG_1, f);
    res = double_to_integer(BIF_P, ceil(f.fd));
    BIF_RET(res);
}
BIF_RETTYPE round_1(BIF_ALIST_1)
{
    Eterm res;
    FloatDef f;
    if (is_not_float(BIF_ARG_1)) {
	if (is_integer(BIF_ARG_1))
	    BIF_RET(BIF_ARG_1);
	BIF_ERROR(BIF_P, BADARG);
    }
    GET_DOUBLE(BIF_ARG_1, f);
    res = double_to_integer(BIF_P, round(f.fd));
    BIF_RET(res);
}
BIF_RETTYPE length_1(BIF_ALIST_1)
{
    Eterm args[3];
    args[0] = BIF_ARG_1;
    args[1] = make_small(0);
    args[2] = BIF_ARG_1;
    return erlang_length_trap(BIF_P, args, A__I);
}
static BIF_RETTYPE erlang_length_trap(BIF_ALIST_3)
{
    Eterm res;
    res = erts_trapping_length_1(BIF_P, BIF__ARGS);
    if (is_value(res)) {
        BIF_RET(res);
    } else {
        if (BIF_P->freason == TRAP) {
            BIF_TRAP3(&erlang_length_export, BIF_P, BIF_ARG_1, BIF_ARG_2, BIF_ARG_3);
        } else {
            ERTS_BIF_ERROR_TRAPPED1(BIF_P, BIF_P->freason,
                                    BIF_TRAP_EXPORT(BIF_length_1), BIF_ARG_3);
        }
    }
}
Eterm erts_trapping_length_1(Process* p, Eterm* args)
{
    Eterm list;
    Uint i;
    Uint max_iter;
    Uint saved_max_iter;
#if defined(DEBUG) || defined(VALGRIND)
    max_iter = 50;
#else
    max_iter = ERTS_BIF_REDS_LEFT(p) * 16;
#endif
    saved_max_iter = max_iter;
    ASSERT(max_iter > 0);
    list = args[0];
    i = unsigned_val(args[1]);
    while (is_list(list) && max_iter != 0) {
	list = CDR(list_val(list));
	i++, max_iter--;
    }
    if (is_list(list)) {
        args[0] = list;
        args[1] = make_small(i);
        p->freason = TRAP;
        BUMP_ALL_REDS(p);
        return THE_NON_VALUE;
    } else if (is_not_nil(list))  {
	BIF_ERROR(p, BADARG);
    }
    BUMP_REDS(p, (saved_max_iter - max_iter) / 16);
    BIF_RET(make_small(i));
}
BIF_RETTYPE size_1(BIF_ALIST_1)
{
    if (is_tuple(BIF_ARG_1)) {
        Eterm* tupleptr = tuple_val(BIF_ARG_1);
        BIF_RET(make_small(arityval(*tupleptr)));
    } else if (is_bitstring(BIF_ARG_1)) {
        Uint sz = BYTE_SIZE(bitstring_size(BIF_ARG_1));
        if (IS_USMALL(0, sz)) {
            BIF_RET(make_small(sz));
        } else {
            Eterm* hp = HeapFragOnlyAlloc(BIF_P, BIG_UINT_HEAP_SIZE);
            BIF_RET(uint_to_big(sz, hp));
        }
    }
    BIF_ERROR(BIF_P, BADARG);
}
BIF_RETTYPE is_integer_3(BIF_ALIST_3)
{
    if(is_not_integer(BIF_ARG_2) ||
       is_not_integer(BIF_ARG_3)) {
        BIF_ERROR(BIF_P, BADARG);
    }
    if(is_not_integer(BIF_ARG_1)) {
        BIF_RET(am_false);
    }
    BIF_RET((CMP_LE(BIF_ARG_2, BIF_ARG_1) && CMP_LE(BIF_ARG_1, BIF_ARG_3)) ?
        am_true : am_false);
}
BIF_RETTYPE bit_size_1(BIF_ALIST_1)
{
    Uint size;
    if (is_not_bitstring(BIF_ARG_1)) {
        BIF_ERROR(BIF_P, BADARG);
    }
    size = bitstring_size(BIF_ARG_1);
    if (IS_USMALL(0, size)) {
        BIF_RET(make_small(size));
    } else {
        Eterm* hp = HeapFragOnlyAlloc(BIF_P, BIG_UINT_HEAP_SIZE);
        BIF_RET(uint_to_big(size, hp));
    }
}
BIF_RETTYPE byte_size_1(BIF_ALIST_1)
{
    Uint size;
    if (is_not_bitstring(BIF_ARG_1)) {
        BIF_ERROR(BIF_P, BADARG);
    }
    size = NBYTES(bitstring_size(BIF_ARG_1));
    if (IS_USMALL(0, size)) {
        BIF_RET(make_small(size));
    } else {
        Eterm* hp = HeapFragOnlyAlloc(BIF_P, BIG_UINT_HEAP_SIZE);
        BIF_RET(uint_to_big(size, hp));
    }
}
static Eterm
double_to_integer(Process* p, double x)
{
    int is_negative;
    int ds;
    ErtsDigit* xp;
    int i;
    Eterm res;
    size_t sz;
    Eterm* hp;
    double dbase;
    if ((x < (double) (MAX_SMALL+1)) && (x > (double) (MIN_SMALL-1))) {
	Sint xi = x;
	return make_small(xi);
    }
    if (x >= 0) {
	is_negative = 0;
    } else {
	is_negative = 1;
	x = -x;
    }
    ds = 0;
    dbase = ((double)(D_MASK)+1);
    while(x >= 1.0) {
	x /= dbase;
	ds++;
    }
    sz = BIG_NEED_SIZE(ds);
    hp = HeapFragOnlyAlloc(p, sz);
    res = make_big(hp);
    xp = (ErtsDigit*) (hp + 1);
    for (i = ds-1; i >= 0; i--) {
	ErtsDigit d;
	x *= dbase;
	d = x;
	xp[i] = d;
	x -= d;
    }
    if (is_negative) {
	*hp = make_neg_bignum_header(sz-1);
    } else {
	*hp = make_pos_bignum_header(sz-1);
    }
    return res;
}
BIF_RETTYPE binary_part_3(BIF_ALIST_3)
{
    return erts_binary_part(BIF_P,BIF_ARG_1,BIF_ARG_2, BIF_ARG_3);
}
BIF_RETTYPE binary_part_2(BIF_ALIST_2)
{
    Eterm *tp;
    if (is_not_tuple(BIF_ARG_2)) {
	goto badarg;
    }
    tp = tuple_val(BIF_ARG_2);
    if (arityval(*tp) != 2) {
	goto badarg;
    }
    return erts_binary_part(BIF_P,BIF_ARG_1,tp[1], tp[2]);
 badarg:
   BIF_ERROR(BIF_P,BADARG);
}
BIF_RETTYPE min_2(BIF_ALIST_2)
{
    return CMP_GT(BIF_ARG_1, BIF_ARG_2) ? BIF_ARG_2 : BIF_ARG_1;
}
BIF_RETTYPE max_2(BIF_ALIST_2)
{
    return CMP_LT(BIF_ARG_1, BIF_ARG_2) ? BIF_ARG_2 : BIF_ARG_1;
}