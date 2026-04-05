#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "sys.h"
#include "erl_vm.h"
#include "global.h"
#define ERTS_WANT_TIMER_WHEEL_API
#include "erl_time.h"
#define ERTS_MAX_CLKTCKS \
    ERTS_MONOTONIC_TO_CLKTCKS(ERTS_MONOTONIC_TIME_MAX)
#define ERTS_CLKTCKS_WEEK \
    ERTS_MONOTONIC_TO_CLKTCKS(ERTS_SEC_TO_MONOTONIC(7*60*60*24))
#ifdef ERTS_ENABLE_LOCK_CHECK
#define ASSERT_NO_LOCKED_LOCKS		erts_lc_check_exact(NULL, 0)
#else
#define ASSERT_NO_LOCKED_LOCKS
#endif
#if 0
#  define ERTS_TW_HARD_DEBUG
#endif
#if 0
#  define ERTS_TW_DEBUG
#endif
#if defined(ERTS_TW_HARD_DEBUG) && !defined(ERTS_TW_DEBUG)
#  define ERTS_TW_DEBUG
#endif
#if defined(DEBUG) && !defined(ERTS_TW_DEBUG)
#  define ERTS_TW_DEBUG
#endif
#undef ERTS_TW_ASSERT
#if defined(ERTS_TW_DEBUG)
#  define ERTS_TW_ASSERT(E) ERTS_ASSERT(E)
#else
#  define ERTS_TW_ASSERT(E) ((void) 1)
#endif
#ifdef ERTS_TW_DEBUG
#  define ERTS_TWHEEL_BUMP_YIELD_LIMIT        500
#else
#  define ERTS_TWHEEL_BUMP_YIELD_LIMIT        10000
#endif
#define ERTS_TW_COST_SLOT                     1
#define ERTS_TW_COST_SLOT_MOVE                5
#define ERTS_TW_COST_TIMEOUT                  100
#define ERTS_TW_SOON_WHEEL_FIRST_SLOT 0
#define ERTS_TW_SOON_WHEEL_END_SLOT \
    (ERTS_TW_SOON_WHEEL_FIRST_SLOT + ERTS_TW_SOON_WHEEL_SIZE)
#define ERTS_TW_SOON_WHEEL_MASK (ERTS_TW_SOON_WHEEL_SIZE-1)
#define ERTS_TW_LATER_WHEEL_SHIFT (ERTS_TW_SOON_WHEEL_BITS - 1)
#define ERTS_TW_LATER_WHEEL_SLOT_SIZE \
    ((ErtsMonotonicTime) (1 << ERTS_TW_LATER_WHEEL_SHIFT))
#define ERTS_TW_LATER_WHEEL_POS_MASK \
    (~((ErtsMonotonicTime) (1 << ERTS_TW_LATER_WHEEL_SHIFT)-1))
#define ERTS_TW_LATER_WHEEL_FIRST_SLOT ERTS_TW_SOON_WHEEL_SIZE
#define ERTS_TW_LATER_WHEEL_END_SLOT \
    (ERTS_TW_LATER_WHEEL_FIRST_SLOT + ERTS_TW_LATER_WHEEL_SIZE)
#define ERTS_TW_LATER_WHEEL_MASK (ERTS_TW_LATER_WHEEL_SIZE-1)
#define ERTS_TW_SCNT_BITS 9
#define ERTS_TW_SCNT_SHIFT
#define ERTS_TW_SCNT_SIZE \
    ((ERTS_TW_SOON_WHEEL_SIZE + ERTS_TW_LATER_WHEEL_SIZE) \
     >> ERTS_TW_SCNT_BITS)
#ifdef __GNUC__
#if ERTS_TW_SOON_WHEEL_BITS < ERTS_TW_SCNT_BITS
#  warning Consider larger soon timer wheel
#endif
#if ERTS_TW_SOON_WHEEL_BITS < ERTS_TW_SCNT_BITS
#  warning Consider larger later timer wheel
#endif
#endif
#if SYS_CLOCK_RESOLUTION == 1
#  define TIW_ITIME 1
#  define TIW_ITIME_IS_CONSTANT
#else
static int tiw_itime;
#  define TIW_ITIME tiw_itime
#endif
const int etp_tw_soon_wheel_size = ERTS_TW_SOON_WHEEL_SIZE;
const ErtsMonotonicTime etp_tw_soon_wheel_mask = ERTS_TW_SOON_WHEEL_MASK;
const int etp_tw_soon_wheel_first_slot = ERTS_TW_SOON_WHEEL_FIRST_SLOT;
const int etp_tw_later_wheel_size = ERTS_TW_LATER_WHEEL_SIZE;
const ErtsMonotonicTime etp_tw_later_wheel_slot_size = ERTS_TW_LATER_WHEEL_SLOT_SIZE;
const int etp_tw_later_wheel_shift = ERTS_TW_LATER_WHEEL_SHIFT;
const ErtsMonotonicTime etp_tw_later_wheel_mask = ERTS_TW_LATER_WHEEL_MASK;
const ErtsMonotonicTime etp_tw_later_wheel_pos_mask = ERTS_TW_LATER_WHEEL_POS_MASK;
const int etp_tw_later_wheel_first_slot = ERTS_TW_LATER_WHEEL_FIRST_SLOT;
struct ErtsTimerWheel_ {
    ErtsTWheelTimer *slots[1
                           + ERTS_TW_SOON_WHEEL_SIZE
                           + ERTS_TW_LATER_WHEEL_SIZE];
    ErtsTWheelTimer **w;
    Sint scnt[ERTS_TW_SCNT_SIZE];
    Sint bump_scnt[ERTS_TW_SCNT_SIZE];
    ErtsMonotonicTime pos;
    Uint nto;
    struct {
	Uint nto;
    } at_once;
    struct {
        ErtsMonotonicTime min_tpos;
        Uint nto;
    } soon;
    struct {
        ErtsMonotonicTime min_tpos;
        int min_tpos_slot;
        ErtsMonotonicTime pos;
        Uint nto;
    } later;
    int yield_slot;
    int yield_slots_left;
    ErtsTWheelTimer sentinel;
    int true_next_timeout_time;
    ErtsMonotonicTime next_timeout_pos;
    ErtsMonotonicTime next_timeout_time;
};
#define ERTS_TW_SLOT_AT_ONCE (-1)
#define ERTS_TW_BUMP_LATER_WHEEL(TIW) \
    ((TIW)->pos + ERTS_TW_LATER_WHEEL_SLOT_SIZE >= (TIW)->later.pos)
static int bump_later_wheel(ErtsTimerWheel *tiw, int *yield_count_p);
#ifdef ERTS_TW_DEBUG
#define ERTS_TW_DBG_VERIFY_EMPTY_SOON_SLOTS(TIW, TO_POS) \
    dbg_verify_empty_soon_slots((TIW), (TO_POS))
#define ERTS_TW_DBG_VERIFY_EMPTY_LATER_SLOTS(TIW, TO_POS) \
    dbg_verify_empty_later_slots((TIW), (TO_POS))
void dbg_verify_empty_soon_slots(ErtsTimerWheel *, ErtsMonotonicTime);
void dbg_verify_empty_later_slots(ErtsTimerWheel *, ErtsMonotonicTime);
#else
#define ERTS_TW_DBG_VERIFY_EMPTY_SOON_SLOTS(TIW, TO_POS)
#define ERTS_TW_DBG_VERIFY_EMPTY_LATER_SLOTS(TIW, TO_POS)
#endif
static ERTS_INLINE int
scnt_get_ix(int slot)
{
    return slot >> ERTS_TW_SCNT_BITS;
}
static ERTS_INLINE void
scnt_inc(Sint *scnt, int slot)
{
    scnt[slot >> ERTS_TW_SCNT_BITS]++;
}
#ifdef ERTS_TW_HARD_DEBUG
static ERTS_INLINE void
scnt_ix_inc(Sint *scnt, int six)
{
    scnt[six]++;
}
#endif
static ERTS_INLINE void
scnt_dec(Sint *scnt, int slot)
{
    scnt[slot >> ERTS_TW_SCNT_BITS]--;
    ERTS_TW_ASSERT(scnt[slot >> ERTS_TW_SCNT_BITS] >= 0);
}
static ERTS_INLINE void
scnt_ix_dec(Sint *scnt, int six)
{
    scnt[six]--;
    ERTS_TW_ASSERT(scnt[six] >= 0);
}
static ERTS_INLINE void
scnt_wheel_next(int *slotp, int *leftp, ErtsMonotonicTime *posp,
                int *sixp, Sint *scnt, int first_slot,
                int end_slot, ErtsMonotonicTime slot_sz)
{
    int slot = *slotp;
    int left = *leftp;
    int ix;
    ERTS_TW_ASSERT(*leftp >= 0);
    left--;
    slot++;
    if (slot == end_slot)
        slot = first_slot;
    ix = slot >> ERTS_TW_SCNT_BITS;
    while (!scnt[ix] && left > 0) {
        int diff, old_slot = slot;
        ix++;
        slot = (ix << ERTS_TW_SCNT_BITS);
        diff = slot - old_slot;
        if (left < diff) {
            slot = old_slot + left;
            diff = left;
        }
        if (slot < end_slot)
            left -= diff;
        else {
            left -= end_slot - old_slot;
            slot = first_slot;
            ix = slot >> ERTS_TW_SCNT_BITS;
        }
    }
    ERTS_TW_ASSERT(left >= -1);
    if (posp)
        *posp += slot_sz * ((ErtsMonotonicTime) (*leftp - left));
    if (sixp)
        *sixp = slot >> ERTS_TW_SCNT_BITS;
    *leftp = left;
    *slotp = slot;
}
static ERTS_INLINE void
scnt_soon_wheel_next(int *slotp, int *leftp, ErtsMonotonicTime *posp,
                    int *sixp, Sint *scnt)
{
    scnt_wheel_next(slotp, leftp, posp, sixp, scnt,
                    ERTS_TW_SOON_WHEEL_FIRST_SLOT,
                    ERTS_TW_SOON_WHEEL_END_SLOT, 1);
}
static ERTS_INLINE void
scnt_later_wheel_next(int *slotp, int *leftp, ErtsMonotonicTime *posp,
                    int *sixp, Sint *scnt)
{
    scnt_wheel_next(slotp, leftp, posp, sixp, scnt,
                    ERTS_TW_LATER_WHEEL_FIRST_SLOT,
                    ERTS_TW_LATER_WHEEL_END_SLOT,
                    ERTS_TW_LATER_WHEEL_SLOT_SIZE);
}
static ERTS_INLINE int
soon_slot(ErtsMonotonicTime soon_pos)
{
    ErtsMonotonicTime slot = soon_pos;
    slot &= ERTS_TW_SOON_WHEEL_MASK;
    ERTS_TW_ASSERT(ERTS_TW_SOON_WHEEL_FIRST_SLOT <= slot);
    ERTS_TW_ASSERT(slot < ERTS_TW_SOON_WHEEL_END_SLOT);
    return (int) slot;
}
static ERTS_INLINE int
later_slot(ErtsMonotonicTime later_pos)
{
    ErtsMonotonicTime slot = later_pos;
    slot >>= ERTS_TW_LATER_WHEEL_SHIFT;
    slot &= ERTS_TW_LATER_WHEEL_MASK;
    slot += ERTS_TW_LATER_WHEEL_FIRST_SLOT;
    ERTS_TW_ASSERT(ERTS_TW_LATER_WHEEL_FIRST_SLOT <= slot);
    ERTS_TW_ASSERT(slot < ERTS_TW_LATER_WHEEL_END_SLOT);
    return (int) slot;
}
#ifdef ERTS_TW_HARD_DEBUG
#define ERTS_HARD_DBG_CHK_WHEELS(TIW, CHK_MIN_TPOS) \
    hrd_dbg_check_wheels((TIW), (CHK_MIN_TPOS))
static void hrd_dbg_check_wheels(ErtsTimerWheel *tiw, int check_min_tpos);
#else
#define ERTS_HARD_DBG_CHK_WHEELS(TIW, CHK_MIN_TPOS)
#endif
static ErtsMonotonicTime
find_next_timeout(ErtsSchedulerData *esdp, ErtsTimerWheel *tiw)
{
    int slot, slots;
    int true_min_timeout = 0;
    ErtsMonotonicTime min_timeout_pos;
    ERTS_TW_ASSERT(tiw->pos + ERTS_TW_LATER_WHEEL_SLOT_SIZE < tiw->later.pos
                   && tiw->later.pos <= tiw->pos + ERTS_TW_SOON_WHEEL_SIZE);
    ERTS_HARD_DBG_CHK_WHEELS(tiw, 0);
    ERTS_TW_ASSERT(tiw->at_once.nto == 0);
    ERTS_TW_ASSERT(tiw->nto == tiw->soon.nto + tiw->later.nto);
    ERTS_TW_ASSERT(tiw->yield_slot == ERTS_TW_SLOT_INACTIVE);
    if (tiw->nto == 0) {
        ErtsMonotonicTime curr_time = erts_get_monotonic_time(esdp);
        tiw->pos = min_timeout_pos = ERTS_MONOTONIC_TO_CLKTCKS(curr_time);
        tiw->later.pos = min_timeout_pos + ERTS_TW_SOON_WHEEL_SIZE;
        tiw->later.pos &= ERTS_TW_LATER_WHEEL_POS_MASK;
	min_timeout_pos += ERTS_CLKTCKS_WEEK;
	goto done;
    }
    ERTS_TW_ASSERT(tiw->soon.nto || tiw->later.nto);
    if (!tiw->soon.nto) {
        ErtsMonotonicTime tpos, min_tpos;
        min_tpos = tiw->later.min_tpos & ERTS_TW_LATER_WHEEL_POS_MASK;
        if (min_tpos <= tiw->later.pos) {
            tpos = tiw->later.pos;
            slots = ERTS_TW_LATER_WHEEL_SIZE;
        }
        else {
            ErtsMonotonicTime tmp;
            tmp = min_tpos - tiw->later.pos;
            tmp /= ERTS_TW_LATER_WHEEL_SLOT_SIZE;
            if (tmp >= ERTS_TW_LATER_WHEEL_SIZE) {
                min_timeout_pos = min_tpos - ERTS_TW_LATER_WHEEL_SLOT_SIZE;
                goto done;
            }
            tpos = min_tpos;
            ERTS_TW_DBG_VERIFY_EMPTY_LATER_SLOTS(tiw, min_tpos);
            slots = ERTS_TW_LATER_WHEEL_SIZE - ((int) tmp);
        }
        slot = later_slot(tpos);
        if (tiw->w[slot])
            true_min_timeout = 1;
        else
            scnt_later_wheel_next(&slot, &slots, &tpos, NULL, tiw->scnt);
        tiw->later.min_tpos = tpos;
        tiw->later.min_tpos_slot = slot;
        ERTS_TW_ASSERT(slot == later_slot(tpos));
        tpos -= ERTS_TW_LATER_WHEEL_SLOT_SIZE;
        min_timeout_pos = tpos;
    }
    else {
        ErtsMonotonicTime tpos;
        min_timeout_pos = tiw->pos + ERTS_TW_SOON_WHEEL_SIZE;
        if (tiw->later.min_tpos > (tiw->later.pos
                                   + 2*ERTS_TW_LATER_WHEEL_SLOT_SIZE)) {
            ERTS_TW_DBG_VERIFY_EMPTY_LATER_SLOTS(
                tiw, 2*ERTS_TW_LATER_WHEEL_SLOT_SIZE);
        }
        else {
            int fslot;
            tpos = tiw->later.pos;
            tpos -= ERTS_TW_LATER_WHEEL_SLOT_SIZE;
            fslot = later_slot(tiw->later.pos);
            if (tiw->w[fslot])
                min_timeout_pos = tpos;
            else {
                tpos += ERTS_TW_LATER_WHEEL_SLOT_SIZE;
                if (tpos < min_timeout_pos) {
                    fslot++;
                    if (fslot == ERTS_TW_LATER_WHEEL_END_SLOT)
                        fslot = ERTS_TW_LATER_WHEEL_FIRST_SLOT;
                    if (tiw->w[fslot])
                        min_timeout_pos = tpos;
                }
            }
        }
        if (tiw->soon.min_tpos <= tiw->pos) {
            tpos = tiw->pos;
            slots = ERTS_TW_SOON_WHEEL_SIZE;
        }
        else {
            ErtsMonotonicTime tmp;
            tmp = tiw->soon.min_tpos - tiw->pos;
            ERTS_TW_ASSERT(ERTS_TW_SOON_WHEEL_SIZE > tmp);
            ERTS_TW_DBG_VERIFY_EMPTY_SOON_SLOTS(tiw, tiw->soon.min_tpos);
            slots = ERTS_TW_SOON_WHEEL_SIZE - ((int) tmp);
            tpos = tiw->soon.min_tpos;
        }
        slot = soon_slot(tpos);
        while (tpos < min_timeout_pos) {
            if (tiw->w[slot]) {
                ERTS_TW_ASSERT(tiw->w[slot]->timeout_pos == tpos);
                min_timeout_pos = tpos;
                break;
            }
            scnt_soon_wheel_next(&slot, &slots, &tpos, NULL, tiw->scnt);
        }
        tiw->soon.min_tpos = min_timeout_pos;
        true_min_timeout = 1;
    }
done: {
        ErtsMonotonicTime min_timeout, timeout_pos_limit;
        timeout_pos_limit = tiw->pos + ERTS_CLKTCKS_WEEK;
        if (min_timeout_pos > timeout_pos_limit) {
            min_timeout_pos = timeout_pos_limit;
            true_min_timeout = 0;
        }
        min_timeout = ERTS_CLKTCKS_TO_MONOTONIC(min_timeout_pos);
        tiw->next_timeout_pos = min_timeout_pos;
        tiw->next_timeout_time = min_timeout;
        tiw->true_next_timeout_time = true_min_timeout;
        ERTS_HARD_DBG_CHK_WHEELS(tiw, 1);
        return min_timeout;
    }
}
static ERTS_INLINE void
insert_timer_into_slot(ErtsTimerWheel *tiw, int slot, ErtsTWheelTimer *p)
{
    ERTS_TW_ASSERT(ERTS_TW_SLOT_AT_ONCE <= slot
                   && slot < ERTS_TW_LATER_WHEEL_END_SLOT);
    p->slot = slot;
    if (!tiw->w[slot]) {
	tiw->w[slot] = p;
	p->next = p;
	p->prev = p;
    }
    else {
	ErtsTWheelTimer *next, *prev;
	next = tiw->w[slot];
	prev = next->prev;
	p->next = next;
	p->prev = prev;
	prev->next = p;
	next->prev = p;
    }
    if (slot == ERTS_TW_SLOT_AT_ONCE)
	tiw->at_once.nto++;
    else {
        ErtsMonotonicTime tpos = p->timeout_pos;
        if (slot < ERTS_TW_SOON_WHEEL_END_SLOT) {
            ERTS_TW_ASSERT(p->timeout_pos < tiw->pos + ERTS_TW_SOON_WHEEL_SIZE);
            tiw->soon.nto++;
            if (tiw->soon.min_tpos > tpos)
                tiw->soon.min_tpos = tpos;
        }
        else {
            ERTS_TW_ASSERT(p->timeout_pos >= tiw->pos + ERTS_TW_SOON_WHEEL_SIZE);
            tiw->later.nto++;
            if (tiw->later.min_tpos > tpos) {
                tiw->later.min_tpos = tpos;
                tiw->later.min_tpos_slot = slot;
            }
        }
        scnt_inc(tiw->scnt, slot);
    }
}
static ERTS_INLINE void
remove_timer(ErtsTimerWheel *tiw, ErtsTWheelTimer *p)
{
    int slot = p->slot;
    int empty_slot;
    ERTS_TW_ASSERT(slot != ERTS_TW_SLOT_INACTIVE);
    ERTS_TW_ASSERT(ERTS_TW_SLOT_AT_ONCE <= slot
                   && slot < ERTS_TW_LATER_WHEEL_END_SLOT);
    if (p->next == p) {
        ERTS_TW_ASSERT(tiw->w[slot] == p);
        tiw->w[slot] = NULL;
        empty_slot = 1;
    }
    else {
        if (tiw->w[slot] == p)
            tiw->w[slot] = p->next;
        p->prev->next = p->next;
        p->next->prev = p->prev;
        empty_slot = 0;
    }
    if (slot == ERTS_TW_SLOT_AT_ONCE) {
	ERTS_TW_ASSERT(tiw->at_once.nto > 0);
	tiw->at_once.nto--;
    }
    else {
        scnt_dec(tiw->scnt, slot);
        if (slot < ERTS_TW_SOON_WHEEL_END_SLOT) {
            if (empty_slot
                && tiw->true_next_timeout_time
                && p->timeout_pos == tiw->next_timeout_pos
                && tiw->yield_slot == ERTS_TW_SLOT_INACTIVE) {
                tiw->true_next_timeout_time = 0;
            }
            if (--tiw->soon.nto == 0)
                tiw->soon.min_tpos = ERTS_MAX_CLKTCKS;
        }
        else {
            if (empty_slot
                && tiw->true_next_timeout_time
                && tiw->later.min_tpos_slot == slot) {
                ErtsMonotonicTime tpos = tiw->later.min_tpos;
                tpos &= ERTS_TW_LATER_WHEEL_POS_MASK;
                tpos -= ERTS_TW_LATER_WHEEL_SLOT_SIZE;
                if (tpos == tiw->next_timeout_pos
                    && tiw->yield_slot == ERTS_TW_SLOT_INACTIVE)
                    tiw->true_next_timeout_time = 0;
            }
            if (--tiw->later.nto == 0) {
                tiw->later.min_tpos = ERTS_MAX_CLKTCKS;
                tiw->later.min_tpos_slot = ERTS_TW_LATER_WHEEL_END_SLOT;
            }
        }
    }
    p->slot = ERTS_TW_SLOT_INACTIVE;
}
ErtsMonotonicTime
erts_check_next_timeout_time(ErtsSchedulerData *esdp)
{
    ErtsTimerWheel *tiw = esdp->timer_wheel;
    ErtsMonotonicTime time;
    ERTS_MSACC_DECLARE_CACHE_X();
    ERTS_TW_ASSERT(tiw->next_timeout_time
                   == ERTS_CLKTCKS_TO_MONOTONIC(tiw->next_timeout_pos));
    if (tiw->true_next_timeout_time)
	return tiw->next_timeout_time;
    if (tiw->next_timeout_pos > tiw->pos + ERTS_TW_SOON_WHEEL_SIZE)
        return tiw->next_timeout_time;
    ERTS_MSACC_PUSH_AND_SET_STATE_CACHED_X(ERTS_MSACC_STATE_TIMERS);
    time = find_next_timeout(esdp, tiw);
    ERTS_MSACC_POP_STATE_M_X();
    return time;
}
static ERTS_INLINE void
timeout_timer(ErtsTWheelTimer *p)
{
    ErlTimeoutProc timeout;
    void *arg;
    p->slot = ERTS_TW_SLOT_INACTIVE;
    timeout = p->timeout;
    arg = p->arg;
    (*timeout)(arg);
    ASSERT_NO_LOCKED_LOCKS;
}
void
erts_bump_timers(ErtsTimerWheel *tiw, ErtsMonotonicTime curr_time)
{
    int slot, restarted, yield_count, slots, scnt_ix;
    ErtsMonotonicTime bump_to;
    Sint *scnt, *bump_scnt;
    ERTS_MSACC_PUSH_AND_SET_STATE_M_X(ERTS_MSACC_STATE_TIMERS);
    yield_count = ERTS_TWHEEL_BUMP_YIELD_LIMIT;
    scnt = &tiw->scnt[0];
    bump_scnt = &tiw->bump_scnt[0];
    slot = tiw->yield_slot;
    restarted = slot != ERTS_TW_SLOT_INACTIVE;
    if (restarted) {
	bump_to = tiw->pos;
        if (slot >= ERTS_TW_LATER_WHEEL_FIRST_SLOT)
            goto restart_yielded_later_slot;
        tiw->yield_slot = ERTS_TW_SLOT_INACTIVE;
        if (slot == ERTS_TW_SLOT_AT_ONCE)
            goto restart_yielded_at_once_slot;
        scnt_ix = scnt_get_ix(slot);
	slots = tiw->yield_slots_left;
        ASSERT(0 <= slots && slots <= ERTS_TW_SOON_WHEEL_SIZE);
        goto restart_yielded_soon_slot;
    }
    do {
        restarted = 0;
	bump_to = ERTS_MONOTONIC_TO_CLKTCKS(curr_time);
        tiw->true_next_timeout_time = 1;
        tiw->next_timeout_pos = bump_to;
        tiw->next_timeout_time = ERTS_CLKTCKS_TO_MONOTONIC(bump_to);
	while (1) {
	    ErtsTWheelTimer *p;
	    if (tiw->nto == 0) {
	    empty_wheel:
                ERTS_TW_DBG_VERIFY_EMPTY_SOON_SLOTS(tiw, bump_to);
                ERTS_TW_DBG_VERIFY_EMPTY_LATER_SLOTS(tiw, bump_to);
		tiw->true_next_timeout_time = 0;
                tiw->next_timeout_pos = bump_to + ERTS_CLKTCKS_WEEK;
		tiw->next_timeout_time = ERTS_CLKTCKS_TO_MONOTONIC(tiw->next_timeout_pos);;
		tiw->pos = bump_to;
                tiw->later.pos = bump_to + ERTS_TW_SOON_WHEEL_SIZE;
                tiw->later.pos &= ERTS_TW_LATER_WHEEL_POS_MASK;
		tiw->yield_slot = ERTS_TW_SLOT_INACTIVE;
                ERTS_MSACC_POP_STATE_M_X();
		return;
	    }
	    p = tiw->w[ERTS_TW_SLOT_AT_ONCE];
	    if (p) {
                if (p->next == p) {
                    ERTS_TW_ASSERT(tiw->sentinel.next == &tiw->sentinel);
                    ERTS_TW_ASSERT(tiw->sentinel.prev == &tiw->sentinel);
                }
                else {
                    tiw->sentinel.next = p->next;
                    tiw->sentinel.prev = p->prev;
                    tiw->sentinel.next->prev = &tiw->sentinel;
                    tiw->sentinel.prev->next = &tiw->sentinel;
                }
                tiw->w[ERTS_TW_SLOT_AT_ONCE] = NULL;
                while (1) {
                    ERTS_TW_ASSERT(tiw->nto > 0);
                    ERTS_TW_ASSERT(tiw->at_once.nto > 0);
                    tiw->nto--;
                    tiw->at_once.nto--;
                    timeout_timer(p);
                    yield_count -= ERTS_TW_COST_TIMEOUT;
                restart_yielded_at_once_slot:
                    p = tiw->sentinel.next;
                    if (p == &tiw->sentinel) {
                        ERTS_TW_ASSERT(tiw->sentinel.prev == &tiw->sentinel);
                        break;
                    }
                    if (yield_count <= 0) {
                        ERTS_TW_ASSERT(tiw->nto > 0);
                        ERTS_TW_ASSERT(tiw->at_once.nto > 0);
                        tiw->yield_slot = ERTS_TW_SLOT_AT_ONCE;
                        ERTS_MSACC_POP_STATE_M_X();
                        return;
                    }
                    tiw->sentinel.next = p->next;
                    p->next->prev = &tiw->sentinel;
                }
	    }
	    if (tiw->pos >= bump_to) {
                if (tiw->at_once.nto)
                    continue;
                ERTS_MSACC_POP_STATE_M_X();
		break;
            }
	    if (tiw->nto == 0)
		goto empty_wheel;
            sys_memcpy((void *) bump_scnt, (void *) scnt,
                       sizeof(Sint) * ERTS_TW_SCNT_SIZE);
	    if (tiw->soon.min_tpos > tiw->pos) {
		ErtsMonotonicTime skip_until_pos = tiw->soon.min_tpos;
		if (skip_until_pos > bump_to)
		    skip_until_pos = bump_to;
		skip_until_pos--;
		if (skip_until_pos > tiw->pos) {
                    ERTS_TW_DBG_VERIFY_EMPTY_SOON_SLOTS(tiw, skip_until_pos);
		    tiw->pos = skip_until_pos;
		}
	    }
            {
                ErtsMonotonicTime tmp_slots = bump_to - tiw->pos;
                if (tmp_slots < ERTS_TW_SOON_WHEEL_SIZE)
                    slots = (int) tmp_slots;
                else
                    slots = ERTS_TW_SOON_WHEEL_SIZE;
            }
            slot = soon_slot(tiw->pos+1);
	    tiw->pos = bump_to;
            tiw->next_timeout_pos = bump_to;
            tiw->next_timeout_time = ERTS_CLKTCKS_TO_MONOTONIC(bump_to);
            scnt_ix = scnt_get_ix(slot);
	    while (slots > 0) {
                yield_count -= ERTS_TW_COST_SLOT;
		p = tiw->w[slot];
		if (p) {
		    if (p->next == p) {
			ERTS_TW_ASSERT(tiw->sentinel.next == &tiw->sentinel);
			ERTS_TW_ASSERT(tiw->sentinel.prev == &tiw->sentinel);
		    }
		    else {
			tiw->sentinel.next = p->next;
			tiw->sentinel.prev = p->prev;
			tiw->sentinel.next->prev = &tiw->sentinel;
			tiw->sentinel.prev->next = &tiw->sentinel;
		    }
		    tiw->w[slot] = NULL;
		    while (1) {
                        ERTS_TW_ASSERT(ERTS_TW_SOON_WHEEL_FIRST_SLOT <= p->slot
                                       && p->slot < ERTS_TW_SOON_WHEEL_END_SLOT);
                        if (--tiw->soon.nto == 0)
                            tiw->soon.min_tpos = ERTS_MAX_CLKTCKS;
                        scnt_ix_dec(scnt, scnt_ix);
                        if (p->timeout_pos <= bump_to) {
                            timeout_timer(p);
                            tiw->nto--;
                            scnt_ix_dec(bump_scnt, scnt_ix);
                            yield_count -= ERTS_TW_COST_TIMEOUT;
                        }
                        else {
                            insert_timer_into_slot(tiw, slot, p);
                            yield_count -= ERTS_TW_COST_SLOT_MOVE;
                        }
		    restart_yielded_soon_slot:
			p = tiw->sentinel.next;
			if (p == &tiw->sentinel) {
			    ERTS_TW_ASSERT(tiw->sentinel.prev == &tiw->sentinel);
			    break;
			}
			if (yield_count <= 0) {
			    tiw->yield_slot = slot;
			    tiw->yield_slots_left = slots;
                            ERTS_MSACC_POP_STATE_M_X();
			    return;
			}
			tiw->sentinel.next = p->next;
			p->next->prev = &tiw->sentinel;
		    }
		}
                scnt_soon_wheel_next(&slot, &slots, NULL, &scnt_ix, bump_scnt);
	    }
            if (ERTS_TW_BUMP_LATER_WHEEL(tiw)) {
            restart_yielded_later_slot:
                if (bump_later_wheel(tiw, &yield_count))
                    return;
            }
	}
    } while (restarted);
    tiw->true_next_timeout_time = 0;
    ERTS_TW_ASSERT(tiw->next_timeout_pos == bump_to);
    (void) find_next_timeout(NULL, tiw);
    ERTS_MSACC_POP_STATE_M_X();
}
static int
bump_later_wheel(ErtsTimerWheel *tiw, int *ycount_p)
{
    ErtsMonotonicTime cpos = tiw->pos;
    ErtsMonotonicTime later_pos = tiw->later.pos;
    int ycount = *ycount_p;
    int slots, fslot, scnt_ix;
    Sint *scnt, *bump_scnt;
    scnt = &tiw->scnt[0];
    bump_scnt = &tiw->bump_scnt[0];
    ERTS_HARD_DBG_CHK_WHEELS(tiw, 0);
    if (tiw->yield_slot >= ERTS_TW_LATER_WHEEL_FIRST_SLOT) {
        fslot = tiw->yield_slot;
        scnt_ix = scnt_get_ix(fslot);
        slots = tiw->yield_slots_left;
        ASSERT(0 <= slots && slots <= ERTS_TW_LATER_WHEEL_SIZE);
        tiw->yield_slot = ERTS_TW_SLOT_INACTIVE;
        goto restart_yielded_slot;
    }
    else {
        ErtsMonotonicTime end_later_pos, tmp_slots, min_tpos;
        min_tpos = tiw->later.min_tpos & ERTS_TW_LATER_WHEEL_POS_MASK;
        end_later_pos = cpos + ERTS_TW_SOON_WHEEL_SIZE;
        end_later_pos &= ERTS_TW_LATER_WHEEL_POS_MASK;
        if (min_tpos > later_pos) {
            if (min_tpos > end_later_pos) {
                ERTS_TW_DBG_VERIFY_EMPTY_LATER_SLOTS(tiw, end_later_pos);
                tiw->later.pos = end_later_pos;
                goto done;
            }
            later_pos = min_tpos;
            ERTS_TW_DBG_VERIFY_EMPTY_LATER_SLOTS(tiw, later_pos);
        }
        tmp_slots = end_later_pos;
        tmp_slots -= later_pos;
        tmp_slots /= ERTS_TW_LATER_WHEEL_SLOT_SIZE;
        if (tmp_slots < ERTS_TW_LATER_WHEEL_SIZE)
            slots = (int) tmp_slots;
        else
            slots = ERTS_TW_LATER_WHEEL_SIZE;
        fslot = later_slot(later_pos);
        scnt_ix = scnt_get_ix(fslot);
        tiw->later.pos = end_later_pos;
    }
    while (slots > 0) {
        ErtsTWheelTimer *p;
        ycount -= ERTS_TW_COST_SLOT;
        p = tiw->w[fslot];
        if (p) {
            if (p->next == p) {
                ERTS_TW_ASSERT(tiw->sentinel.next == &tiw->sentinel);
                ERTS_TW_ASSERT(tiw->sentinel.prev == &tiw->sentinel);
            }
            else {
                tiw->sentinel.next = p->next;
                tiw->sentinel.prev = p->prev;
                tiw->sentinel.next->prev = &tiw->sentinel;
                tiw->sentinel.prev->next = &tiw->sentinel;
            }
            tiw->w[fslot] = NULL;
            while (1) {
                ErtsMonotonicTime tpos = p->timeout_pos;
                ERTS_TW_ASSERT(p->slot == fslot);
                if (--tiw->later.nto == 0) {
                    tiw->later.min_tpos = ERTS_MAX_CLKTCKS;
                    tiw->later.min_tpos_slot = ERTS_TW_LATER_WHEEL_END_SLOT;
                }
                scnt_ix_dec(scnt, scnt_ix);
                if (tpos >= tiw->later.pos + ERTS_TW_LATER_WHEEL_SLOT_SIZE) {
                    insert_timer_into_slot(tiw, fslot, p);
                    ycount -= ERTS_TW_COST_SLOT_MOVE;
                }
                else {
                    scnt_ix_dec(bump_scnt, scnt_ix);
                    ERTS_TW_ASSERT(tpos < cpos + ERTS_TW_SOON_WHEEL_SIZE);
                    if (tpos > cpos) {
                        insert_timer_into_slot(tiw, soon_slot(tpos), p);
                        ycount -= ERTS_TW_COST_SLOT_MOVE;
                    }
                    else {
                        timeout_timer(p);
                        tiw->nto--;
                        ycount -= ERTS_TW_COST_TIMEOUT;
                    }
                }
            restart_yielded_slot:
                p = tiw->sentinel.next;
                if (p == &tiw->sentinel) {
                    ERTS_TW_ASSERT(tiw->sentinel.prev == &tiw->sentinel);
                    break;
                }
                if (ycount < 0) {
                    tiw->yield_slot = fslot;
                    tiw->yield_slots_left = slots;
                    *ycount_p = 0;
                    ERTS_HARD_DBG_CHK_WHEELS(tiw, 0);
                    return 1;
                }
                tiw->sentinel.next = p->next;
                p->next->prev = &tiw->sentinel;
            }
        }
        scnt_later_wheel_next(&fslot, &slots, NULL, &scnt_ix, bump_scnt);
    }
done:
    ERTS_HARD_DBG_CHK_WHEELS(tiw, 0);
    *ycount_p = ycount;
    return 0;
}
Uint
erts_timer_wheel_memory_size(void)
{
    return sizeof(ErtsTimerWheel)*erts_no_schedulers;
}
ErtsTimerWheel *
erts_create_timer_wheel(ErtsSchedulerData *esdp)
{
    ErtsMonotonicTime mtime;
    int i;
    ErtsTimerWheel *tiw;
    ERTS_CT_ASSERT(ERTS_TW_SLOT_AT_ONCE == -1);
    ERTS_CT_ASSERT(ERTS_TW_SLOT_INACTIVE < ERTS_TW_SLOT_AT_ONCE);
    ERTS_CT_ASSERT(ERTS_TW_SLOT_AT_ONCE + 1 == ERTS_TW_SOON_WHEEL_FIRST_SLOT);
    ERTS_CT_ASSERT(ERTS_TW_SOON_WHEEL_FIRST_SLOT < ERTS_TW_SOON_WHEEL_END_SLOT);
    ERTS_CT_ASSERT(ERTS_TW_SOON_WHEEL_END_SLOT == ERTS_TW_LATER_WHEEL_FIRST_SLOT);
    ERTS_CT_ASSERT(ERTS_TW_LATER_WHEEL_FIRST_SLOT < ERTS_TW_LATER_WHEEL_END_SLOT);
    ERTS_CT_ASSERT(ERTS_TW_SOON_WHEEL_SIZE
                   && !(ERTS_TW_SOON_WHEEL_SIZE & (ERTS_TW_SOON_WHEEL_SIZE-1)));
    ERTS_CT_ASSERT(ERTS_TW_LATER_WHEEL_SIZE
                   && !(ERTS_TW_LATER_WHEEL_SIZE & (ERTS_TW_LATER_WHEEL_SIZE-1)));
    tiw = erts_alloc_permanent_cache_aligned(ERTS_ALC_T_TIMER_WHEEL,
					     sizeof(ErtsTimerWheel));
    tiw->w = &tiw->slots[1];
    for(i = ERTS_TW_SLOT_AT_ONCE; i < ERTS_TW_LATER_WHEEL_END_SLOT; i++)
	tiw->w[i] = NULL;
    for (i = 0; i < ERTS_TW_SCNT_SIZE; i++)
        tiw->scnt[i] = 0;
    mtime = erts_get_monotonic_time(esdp);
    tiw->pos = ERTS_MONOTONIC_TO_CLKTCKS(mtime);
    tiw->nto = 0;
    tiw->at_once.nto = 0;
    tiw->soon.min_tpos = ERTS_MAX_CLKTCKS;
    tiw->soon.nto = 0;
    tiw->later.min_tpos = ERTS_MAX_CLKTCKS;
    tiw->later.min_tpos_slot = ERTS_TW_LATER_WHEEL_END_SLOT;
    tiw->later.pos = tiw->pos + ERTS_TW_SOON_WHEEL_SIZE;
    tiw->later.pos &= ERTS_TW_LATER_WHEEL_POS_MASK;
    tiw->later.nto = 0;
    tiw->yield_slot = ERTS_TW_SLOT_INACTIVE;
    tiw->true_next_timeout_time = 0;
    tiw->next_timeout_pos = tiw->pos + ERTS_CLKTCKS_WEEK;
    tiw->next_timeout_time = ERTS_CLKTCKS_TO_MONOTONIC(tiw->next_timeout_pos);
    tiw->sentinel.next = &tiw->sentinel;
    tiw->sentinel.prev = &tiw->sentinel;
    tiw->sentinel.timeout = NULL;
    tiw->sentinel.arg = NULL;
    return tiw;
}
ErtsNextTimeoutRef
erts_get_next_timeout_reference(ErtsTimerWheel *tiw)
{
    return (ErtsNextTimeoutRef) &tiw->next_timeout_time;
}
void
erts_init_time(int time_correction, ErtsTimeWarpMode time_warp_mode)
{
    int itime;
    itime = erts_init_time_sup(time_correction, time_warp_mode);
#ifdef TIW_ITIME_IS_CONSTANT
    if (itime != TIW_ITIME) {
	erts_exit(ERTS_ABORT_EXIT, "timer resolution mismatch %d != %d", itime, TIW_ITIME);
    }
#else
    tiw_itime = itime;
#endif
}
void
erts_twheel_set_timer(ErtsTimerWheel *tiw,
		      ErtsTWheelTimer *p, ErlTimeoutProc timeout,
		      void *arg, ErtsMonotonicTime timeout_pos)
{
    int slot;
    ERTS_MSACC_PUSH_AND_SET_STATE_M_X(ERTS_MSACC_STATE_TIMERS);
    p->timeout = timeout;
    p->arg = arg;
    ERTS_TW_ASSERT(p->slot == ERTS_TW_SLOT_INACTIVE);
    tiw->nto++;
    if (timeout_pos <= tiw->pos) {
        p->timeout_pos = timeout_pos = tiw->pos;
        slot = ERTS_TW_SLOT_AT_ONCE;
    }
    else if (timeout_pos < tiw->pos + ERTS_TW_SOON_WHEEL_SIZE) {
        p->timeout_pos = timeout_pos;
        slot = soon_slot(timeout_pos);
        if (tiw->soon.min_tpos > timeout_pos)
            tiw->soon.min_tpos = timeout_pos;
    }
    else {
        p->timeout_pos = timeout_pos;
        slot = later_slot(timeout_pos);
        timeout_pos &= ERTS_TW_LATER_WHEEL_POS_MASK;
        timeout_pos -= ERTS_TW_LATER_WHEEL_SLOT_SIZE;
    }
    insert_timer_into_slot(tiw, slot, p);
    if (timeout_pos <= tiw->next_timeout_pos) {
	tiw->true_next_timeout_time = 1;
        if (timeout_pos < tiw->next_timeout_pos) {
            tiw->next_timeout_pos = timeout_pos;
            tiw->next_timeout_time = ERTS_CLKTCKS_TO_MONOTONIC(timeout_pos);
        }
    }
    ERTS_MSACC_POP_STATE_M_X();
}
void
erts_twheel_cancel_timer(ErtsTimerWheel *tiw, ErtsTWheelTimer *p)
{
    if (p->slot != ERTS_TW_SLOT_INACTIVE) {
        ERTS_MSACC_PUSH_AND_SET_STATE_M_X(ERTS_MSACC_STATE_TIMERS);
	remove_timer(tiw, p);
        tiw->nto--;
        ERTS_MSACC_POP_STATE_M_X();
    }
}
void
erts_twheel_debug_foreach(ErtsTimerWheel *tiw,
			  void (*tclbk)(void *),
			  void (*func)(void *,
				       ErtsMonotonicTime,
				       void *),
			  void *arg)
{
    ErtsTWheelTimer *tmr;
    int ix;
    tmr = tiw->sentinel.next;
    while (tmr != &tiw->sentinel) {
	if (tmr->timeout == tclbk)
	    (*func)(arg, tmr->timeout_pos, tmr->arg);
	tmr = tmr->next;
    }
    for (ix = ERTS_TW_SLOT_AT_ONCE; ix < ERTS_TW_LATER_WHEEL_END_SLOT; ix++) {
	tmr = tiw->w[ix];
	if (tmr) {
	    do {
		if (tmr->timeout == tclbk)
		    (*func)(arg, tmr->timeout_pos, tmr->arg);
		tmr = tmr->next;
	    } while (tmr != tiw->w[ix]);
	}
    }
}
#ifdef ERTS_TW_DEBUG
void
dbg_verify_empty_soon_slots(ErtsTimerWheel *tiw, ErtsMonotonicTime to_pos)
{
    int ix;
    ErtsMonotonicTime tmp;
    ix = soon_slot(tiw->pos);
    tmp = to_pos;
    if (tmp > tiw->pos) {
        int slots;
        tmp -= tiw->pos;
        ERTS_TW_ASSERT(tmp > 0);
        if (tmp < (ErtsMonotonicTime) ERTS_TW_SOON_WHEEL_SIZE)
            slots = (int) tmp;
        else
            slots = ERTS_TW_SOON_WHEEL_SIZE;
        while (slots > 0) {
            ERTS_TW_ASSERT(!tiw->w[ix]);
            ix++;
            if (ix == ERTS_TW_SOON_WHEEL_END_SLOT)
                ix = ERTS_TW_SOON_WHEEL_FIRST_SLOT;
            slots--;
        }
    }
}
void
dbg_verify_empty_later_slots(ErtsTimerWheel *tiw, ErtsMonotonicTime to_pos)
{
    int ix;
    ErtsMonotonicTime tmp;
    ix = later_slot(tiw->later.pos);
    tmp = to_pos;
    tmp &= ERTS_TW_LATER_WHEEL_POS_MASK;
    if (tmp > tiw->later.pos) {
        ErtsMonotonicTime pos_min;
        int slots;
        tmp -= tiw->later.pos;
        tmp /= ERTS_TW_LATER_WHEEL_SLOT_SIZE;
        ERTS_TW_ASSERT(tmp > 0);
        pos_min = tiw->later.pos;
        if (tmp < (ErtsMonotonicTime) ERTS_TW_LATER_WHEEL_SIZE)
            slots = (int) tmp;
        else {
            pos_min += ((tmp / ERTS_TW_LATER_WHEEL_SIZE)
                        * ERTS_TW_LATER_WHEEL_SLOT_SIZE);
            slots = ERTS_TW_LATER_WHEEL_SIZE;
        }
        while (slots > 0) {
            ErtsTWheelTimer *tmr = tiw->w[ix];
            pos_min += ERTS_TW_LATER_WHEEL_SLOT_SIZE;
            if (tmr) {
                ErtsTWheelTimer *end = tmr;
                do {
                    ERTS_TW_ASSERT(tmr->timeout_pos >= pos_min);
                    tmr = tmr->next;
                } while (tmr != end);
            }
            ix++;
            if (ix == ERTS_TW_LATER_WHEEL_END_SLOT)
                ix = ERTS_TW_LATER_WHEEL_FIRST_SLOT;
            slots--;
        }
    }
}
#endif
#ifdef ERTS_TW_HARD_DEBUG
static void
hrd_dbg_check_wheels(ErtsTimerWheel *tiw, int check_min_tpos)
{
    int ix, six, soon_tmo, later_tmo, at_once_tmo,
        scnt_slot, scnt_slots, scnt_six;
    ErtsMonotonicTime min_tpos;
    Sint scnt[ERTS_TW_SCNT_SIZE];
    ErtsTWheelTimer *p;
    for (six = 0; six < ERTS_TW_SCNT_SIZE; six++)
        scnt[six] = 0;
    min_tpos = ERTS_MONOTONIC_TO_CLKTCKS(tiw->next_timeout_time);
    at_once_tmo = 0;
    p = tiw->w[ERTS_TW_SLOT_AT_ONCE];
    if (p) {
        ErtsTWheelTimer *first = p;
        do {
            at_once_tmo++;
            ERTS_TW_ASSERT(p->slot == ERTS_TW_SLOT_AT_ONCE);
            ERTS_TW_ASSERT(p->timeout_pos <= tiw->pos);
            ERTS_TW_ASSERT(!check_min_tpos || tiw->pos >= min_tpos);
            ERTS_TW_ASSERT(p->next->prev == p);
            p = p->next;
        } while (p != first);
    }
    soon_tmo = 0;
    scnt_slot = ERTS_TW_SOON_WHEEL_END_SLOT-1;
    scnt_slots = ERTS_TW_SOON_WHEEL_SIZE;
    scnt_six = 0;
    scnt_soon_wheel_next(&scnt_slot, &scnt_slots,
                         NULL, &scnt_six, tiw->scnt);
    for (ix = ERTS_TW_SOON_WHEEL_FIRST_SLOT;
         ix < ERTS_TW_SOON_WHEEL_END_SLOT;
         ix++) {
        p = tiw->w[ix];
        six = scnt_get_ix(ix);
        ERTS_TW_ASSERT(!p || six == scnt_six);
        if (p) {
            ErtsTWheelTimer *first = p;
            do {
                ErtsMonotonicTime tpos = p->timeout_pos;
                soon_tmo++;
                scnt_ix_inc(scnt, six);
                ERTS_TW_ASSERT(p->slot == ix);
                ERTS_TW_ASSERT(ix == soon_slot(tpos));
                ERTS_TW_ASSERT(p->timeout_pos < tiw->pos + ERTS_TW_SOON_WHEEL_SIZE);
                ERTS_TW_ASSERT(!check_min_tpos || tpos >= min_tpos);
                ERTS_TW_ASSERT(p->next->prev == p);
                p = p->next;
            } while (p != first);
        }
        if (ix == scnt_slot)
            scnt_soon_wheel_next(&scnt_slot, &scnt_slots,
                                 NULL, &scnt_six, tiw->scnt);
    }
    later_tmo = 0;
    scnt_slot = ERTS_TW_SOON_WHEEL_END_SLOT-1;
    scnt_slots = ERTS_TW_SOON_WHEEL_SIZE;
    scnt_six = 0;
    scnt_later_wheel_next(&scnt_slot, &scnt_slots,
                         NULL, &scnt_six, tiw->scnt);
    for (ix = ERTS_TW_LATER_WHEEL_FIRST_SLOT;
         ix < ERTS_TW_LATER_WHEEL_END_SLOT;
         ix++) {
        p = tiw->w[ix];
        six = scnt_get_ix(ix);
        ERTS_TW_ASSERT(!p || six == scnt_six);
        if (p) {
            ErtsTWheelTimer *first = p;
            six = scnt_get_ix(ix);
            do {
                ErtsMonotonicTime tpos = p->timeout_pos;
                later_tmo++;
                scnt_ix_inc(scnt, six);
                ERTS_TW_ASSERT(p->slot == ix);
                ERTS_TW_ASSERT(later_slot(tpos) == ix);
                tpos &= ERTS_TW_LATER_WHEEL_POS_MASK;
                tpos -= ERTS_TW_LATER_WHEEL_SLOT_SIZE;
                ERTS_TW_ASSERT(!check_min_tpos || tpos >= min_tpos);
                ERTS_TW_ASSERT(p->next->prev == p);
                p = p->next;
            } while (p != first);
        }
        if (ix == scnt_slot)
            scnt_later_wheel_next(&scnt_slot, &scnt_slots,
                                NULL, &scnt_six, tiw->scnt);
    }
    if (tiw->yield_slot != ERTS_TW_SLOT_INACTIVE) {
        p = tiw->sentinel.next;
        ix = tiw->yield_slot;
        while (p != &tiw->sentinel) {
            ErtsMonotonicTime tpos = p->timeout_pos;
            ERTS_TW_ASSERT(ix == p->slot);
            if (ix == ERTS_TW_SLOT_AT_ONCE)
                at_once_tmo++;
            else {
                scnt_inc(scnt, ix);
                if (ix >= ERTS_TW_LATER_WHEEL_FIRST_SLOT) {
                    later_tmo++;
                    ERTS_TW_ASSERT(ix == later_slot(tpos));
                }
                else {
                    soon_tmo++;
                    ERTS_TW_ASSERT(ix == (tpos & ERTS_TW_SOON_WHEEL_MASK));
                    ERTS_TW_ASSERT(tpos < tiw->pos + ERTS_TW_SOON_WHEEL_SIZE);
                }
                p = p->next;
            }
        }
    }
    ERTS_TW_ASSERT(tiw->at_once.nto == at_once_tmo);
    ERTS_TW_ASSERT(tiw->soon.nto == soon_tmo);
    ERTS_TW_ASSERT(tiw->later.nto == later_tmo);
    ERTS_TW_ASSERT(tiw->nto == soon_tmo + later_tmo + at_once_tmo);
    for (six = 0; six < ERTS_TW_SCNT_SIZE; six++)
        ERTS_TW_ASSERT(scnt[six] == tiw->scnt[six]);
}
#endif