#define ETHR_INLINE_FUNC_NAME_(X) X ## __
#define ETHR_EVENT_IMPL__
#include "ethread.h"
#include "ethr_internal.h"
void
ethr_init_event__(void)
{
}
int
ethr_event_init(ethr_event *e)
{
    ethr_atomic32_init(&e->state, ETHR_EVENT_OFF__);
    e->handle = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (e->handle == INVALID_HANDLE_VALUE)
	return ethr_win_get_errno__();
    return 0;
}
int
ethr_event_prepare_timed(ethr_event *e)
{
    return 0;
}
int
ethr_event_destroy(ethr_event *e)
{
    BOOL res = CloseHandle(e->handle);
    return res == 0 ? ethr_win_get_errno__() : 0;
}
void
ethr_event_set(ethr_event *e)
{
    ethr_event_set__(e);
}
void
ethr_event_reset(ethr_event *e)
{
    ethr_event_reset__(e);
}
#define MILLISECONDS_PER_WEEK__ (7*24*60*60*1000)
static ETHR_INLINE int
wait(ethr_event *e, int spincount, ethr_sint64_t timeout)
{
    DWORD code, tmo;
    int sc, res, until_yield = ETHR_YIELD_AFTER_BUSY_LOOPS;
    int timeout_res = ETIMEDOUT;
    if (timeout < 0)
	tmo = INFINITE;
    else if (timeout == 0) {
	ethr_sint32_t state = ethr_atomic32_read(&e->state);
	if (state == ETHR_EVENT_ON__) {
	    ETHR_MEMBAR(ETHR_LoadLoad|ETHR_LoadStore);
	    return 0;
	}
	return ETIMEDOUT;
    }
    else {
        ethr_sint64_t tmo_ms;
	tmo_ms = (timeout - 1) / (1000*1000) + 1;
        if (tmo_ms <= MILLISECONDS_PER_WEEK__) {
            tmo = (DWORD) tmo_ms;
        }
        else {
            tmo = MILLISECONDS_PER_WEEK__;
            timeout_res = EINTR;
        }
    }
    if (spincount < 0)
	ETHR_FATAL_ERROR__(EINVAL);
    sc = spincount;
    while (1) {
	ethr_sint32_t state;
	while (1) {
	    state = ethr_atomic32_read(&e->state);
	    if (state == ETHR_EVENT_ON__) {
		ETHR_MEMBAR(ETHR_LoadLoad|ETHR_LoadStore);
		return 0;
	    }
	    if (sc == 0)
		break;
	    sc--;
	    ETHR_SPIN_BODY;
	    if (--until_yield == 0) {
		until_yield = ETHR_YIELD_AFTER_BUSY_LOOPS;
		res = ETHR_YIELD();
		if (res != 0)
		    ETHR_FATAL_ERROR__(res);
	    }
	}
	if (state != ETHR_EVENT_OFF_WAITER__) {
	    state = ethr_atomic32_cmpxchg(&e->state,
					  ETHR_EVENT_OFF_WAITER__,
					  ETHR_EVENT_OFF__);
	    if (state == ETHR_EVENT_ON__)
		return 0;
	    ETHR_ASSERT(state == ETHR_EVENT_OFF__);
	}
	code = WaitForSingleObject(e->handle, tmo);
        if (code == WAIT_TIMEOUT)
            return timeout_res;
	if (code != WAIT_OBJECT_0)
	    ETHR_FATAL_ERROR__(ethr_win_get_errno__());
    }
}
int
ethr_event_wait(ethr_event *e)
{
    return wait(e, 0, -1);
}
int
ethr_event_swait(ethr_event *e, int spincount)
{
    return wait(e, spincount, -1);
}
int
ethr_event_twait(ethr_event *e, ethr_sint64_t timeout)
{
    return wait(e, 0, timeout);
}
int
ethr_event_stwait(ethr_event *e, int spincount, ethr_sint64_t timeout)
{
    return wait(e, spincount, timeout);
}