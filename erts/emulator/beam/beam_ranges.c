#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "sys.h"
#include "erl_vm.h"
#include "global.h"
#include "beam_code.h"
#include "erl_unicode.h"
typedef struct {
    ErtsCodePtr start;
    erts_atomic_t end;
} Range;
ErtsLiteralArea** erts_dump_lit_areas;
Uint erts_dump_num_lit_areas;
#define RANGE_END(R) ((ErtsCodePtr)erts_atomic_read_nob(&(R)->end))
static Range* find_range(ErtsCodePtr pc);
static void lookup_loc(FunctionInfo* fi, ErtsCodePtr pc,
                       const BeamCodeHeader*, int idx);
struct ranges {
    Range* modules;
    Sint n;
    Sint allocated;
    erts_atomic_t mid;
};
static struct ranges r[ERTS_NUM_CODE_IX];
static erts_atomic_t mem_used;
static Range* write_ptr;
#ifdef HARD_DEBUG
static void check_consistency(struct ranges* p)
{
    int i;
    ASSERT(p->n <= p->allocated);
    ASSERT((Uint)(p->mid - p->modules) < p->n ||
	   (p->mid == p->modules && p->n == 0));
    for (i = 0; i < p->n; i++) {
	ASSERT(p->modules[i].start <= RANGE_END(&p->modules[i]));
	ASSERT(!i || RANGE_END(&p->modules[i-1]) < p->modules[i].start);
    }
}
#  define CHECK(r) check_consistency(r)
#else
#  define CHECK(r)
#endif
static int
rangecompare(Range* a, Range* b)
{
    if (a->start < b->start) {
	return -1;
    } else if (a->start == b->start) {
	return 0;
    } else {
	return 1;
    }
}
void
erts_init_ranges(void)
{
    Sint i;
    erts_atomic_init_nob(&mem_used, 0);
    for (i = 0; i < ERTS_NUM_CODE_IX; i++) {
	r[i].modules = 0;
	r[i].n = 0;
	r[i].allocated = 0;
	erts_atomic_init_nob(&r[i].mid, 0);
    }
    erts_dump_num_lit_areas = 8;
    erts_dump_lit_areas = (ErtsLiteralArea **)
        erts_alloc(ERTS_ALC_T_CRASH_DUMP,
                   erts_dump_num_lit_areas * sizeof(ErtsLiteralArea*));
}
void
erts_start_staging_ranges(int num_new)
{
    ErtsCodeIndex src = erts_active_code_ix();
    ErtsCodeIndex dst = erts_staging_code_ix();
    Sint need;
    if (r[dst].modules) {
	erts_atomic_add_nob(&mem_used, -r[dst].allocated);
	erts_free(ERTS_ALC_T_MODULE_REFS, r[dst].modules);
    }
    need = r[dst].allocated = r[src].n + num_new;
    erts_atomic_add_nob(&mem_used, need);
    write_ptr = erts_alloc(ERTS_ALC_T_MODULE_REFS,
			   need * sizeof(Range));
    r[dst].modules = write_ptr;
}
void
erts_end_staging_ranges(int commit)
{
    if (commit) {
	Sint i;
	ErtsCodeIndex src = erts_active_code_ix();
	ErtsCodeIndex dst = erts_staging_code_ix();
	Range* mp;
	Sint num_inserted;
	mp = r[dst].modules;
	num_inserted = write_ptr - mp;
	for (i = 0; i < r[src].n; i++) {
	    Range* rp = r[src].modules+i;
	    if (rp->start < RANGE_END(rp)) {
		write_ptr->start = rp->start;
		erts_atomic_init_nob(&write_ptr->end,
					 (erts_aint_t)(RANGE_END(rp)));
		write_ptr++;
	    }
	}
	r[dst].n = write_ptr - mp;
	if (num_inserted > 1) {
	    qsort(mp, r[dst].n, sizeof(Range),
		  (int (*)(const void *, const void *)) rangecompare);
	} else if (num_inserted == 1) {
	    Range t = mp[0];
	    for (i = 0; i < r[dst].n-1 && t.start > mp[i+1].start; i++) {
		mp[i] = mp[i+1];
	    }
	    mp[i] = t;
	}
	r[dst].modules = mp;
	CHECK(&r[dst]);
	erts_atomic_set_nob(&r[dst].mid,
				(erts_aint_t) (r[dst].modules +
					       r[dst].n / 2));
        if (r[dst].allocated > erts_dump_num_lit_areas) {
            erts_dump_num_lit_areas = r[dst].allocated * 2;
            erts_dump_lit_areas = (ErtsLiteralArea **)
                erts_realloc(ERTS_ALC_T_CRASH_DUMP,
                             (void *) erts_dump_lit_areas,
                             erts_dump_num_lit_areas * sizeof(ErtsLiteralArea*));
        }
    }
}
void
erts_update_ranges(const BeamCodeHeader* code, Uint size)
{
    ErtsCodeIndex dst = erts_staging_code_ix();
    ErtsCodeIndex src = erts_active_code_ix();
    if (src == dst) {
	ASSERT(!erts_initialized);
	if (r[dst].modules == NULL) {
	    Sint need = 128;
	    erts_atomic_add_nob(&mem_used, need);
	    r[dst].modules = erts_alloc(ERTS_ALC_T_MODULE_REFS,
					need * sizeof(Range));
	    r[dst].allocated = need;
	    write_ptr = r[dst].modules;
	}
    }
    ASSERT(r[dst].modules);
    write_ptr->start = code;
    erts_atomic_init_nob(&(write_ptr->end),
			     (erts_aint_t)(((byte *)code) + size));
    write_ptr++;
}
void
erts_remove_from_ranges(const BeamCodeHeader* code)
{
    Range* rp = find_range(code);
    erts_atomic_set_nob(&rp->end, (erts_aint_t)rp->start);
}
UWord
erts_ranges_sz(void)
{
    return erts_atomic_read_nob(&mem_used) * sizeof(Range);
}
void
erts_lookup_function_info(FunctionInfo* fi, ErtsCodePtr pc, int full_info)
{
    const ErtsCodeInfo * const *low;
    const ErtsCodeInfo * const *high;
    const ErtsCodeInfo * const *mid;
    const BeamCodeHeader *hdr;
    Range* rp;
    fi->mfa = NULL;
    fi->needed = 5;
    fi->loc = LINE_INVALID_LOCATION;
    rp = find_range(pc);
    if (rp == 0) {
	return;
    }
    hdr = (BeamCodeHeader*) rp->start;
    low = hdr->functions;
    high = low + hdr->num_functions;
    while (low < high) {
	mid = low + (high-low) / 2;
	if (pc < (ErtsCodePtr)(mid[0])) {
	    high = mid;
	} else if (pc < (ErtsCodePtr)(mid[1])) {
	    fi->mfa = &mid[0]->mfa;
	    if (full_info) {
		const ErtsCodeInfo * const *fp = hdr->functions;
		int idx = mid - fp;
		lookup_loc(fi, pc, hdr, idx);
	    }
	    return;
	} else {
	    low = mid + 1;
	}
    }
}
void
erts_set_current_function(FunctionInfo* fi, const ErtsCodeMFA* mfa)
{
    fi->mfa = mfa;
    fi->needed = 5;
    fi->loc = LINE_INVALID_LOCATION;
}
const ErtsCodeMFA*
erts_find_function_from_pc(ErtsCodePtr pc)
{
    FunctionInfo fi;
    erts_lookup_function_info(&fi, pc, 0);
    return fi.mfa;
}
static Range*
find_range(ErtsCodePtr pc)
{
    ErtsCodeIndex active = erts_active_code_ix();
    Range* low = r[active].modules;
    Range* high = low + r[active].n;
    Range* mid = (Range *) erts_atomic_read_nob(&r[active].mid);
    CHECK(&r[active]);
    while (low < high) {
	if (pc < mid->start) {
	    high = mid;
	} else if (pc >= RANGE_END(mid)) {
	    low = mid + 1;
	} else {
	    erts_atomic_set_nob(&r[active].mid, (erts_aint_t) mid);
	    return mid;
	}
	mid = low + (high-low) / 2;
    }
    return 0;
}
static void
lookup_loc(FunctionInfo* fi, const void* pc,
           const BeamCodeHeader* code_hdr, int idx)
{
    const BeamCodeLineTab *lt = code_hdr->line_table;
    const void** low;
    const void** high;
    const void** mid;
    if (lt == NULL) {
	return;
    }
    fi->fname_ptr = lt->fname_ptr;
    low = lt->func_tab[idx];
    high = lt->func_tab[idx+1];
    while (high > low) {
	mid = low + (high-low) / 2;
	if (pc < mid[0]) {
	    high = mid;
	} else if (pc < mid[1]) {
	    int index = mid - lt->func_tab[0];
	    if (lt->loc_size == 2) {
		fi->loc = lt->loc_tab.p2[index];
	    } else {
		ASSERT(lt->loc_size == 4);
		fi->loc = lt->loc_tab.p4[index];
	    }
	    if (fi->loc == LINE_INVALID_LOCATION) {
		return;
	    }
	    fi->needed += 3+2+3+2;
	    return;
	} else {
	    low = mid + 1;
	}
    }
}
ErtsCodePtr
erts_find_next_code_for_line(const BeamCodeHeader* code_hdr,
                             unsigned int line,
                             unsigned int *start_from)
{
    const BeamCodeLineTab *lt = code_hdr->line_table;
    const UWord num_functions = code_hdr->num_functions;
    unsigned int line_index = -1;
    unsigned int num_lines;
    if (lt == NULL) {
	return NULL;
    }
    num_lines = lt->func_tab[num_functions] - lt->func_tab[0];
    if (lt->loc_size == 2) {
        for(unsigned int i=*start_from; i<num_lines; i++) {
            int curr_line = LOC_LINE(lt->loc_tab.p2[i]);
            if (curr_line == line) {
                line_index = i;
                *start_from = i+1;
                break;
            }
        }
    } else {
        for(unsigned int i=*start_from; i<num_lines; i++) {
            int curr_line = LOC_LINE(lt->loc_tab.p4[i]);
            if (curr_line == line) {
                line_index = i;
                *start_from = i+1;
                break;
            }
        }
    }
    if (line_index == -1) {
        *start_from = 0;
        return NULL;
    }
    return lt->func_tab[0][line_index];
}