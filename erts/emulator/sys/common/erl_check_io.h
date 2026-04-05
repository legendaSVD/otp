#ifndef ERL_CHECK_IO_H__
#define ERL_CHECK_IO_H__
#include "sys.h"
#include "erl_sys_driver.h"
struct erts_poll_thread;
Uint erts_check_io_size(void);
Eterm erts_check_io_info(void *proc);
void erts_io_notify_port_task_executed(ErtsPortTaskType type,
                                       ErtsPortTaskHandle *handle,
                                       void (*reset)(ErtsPortTaskHandle *));
#if ERTS_POLL_USE_SCHEDULER_POLLING
Eterm erts_io_handle_nif_select(ErtsMessage *sig);
void erts_io_clear_nif_select_handles(ErtsSchedulerData *esdp);
#endif
int erts_check_io_max_files(void);
void erts_check_io(struct erts_poll_thread *pt, ErtsMonotonicTime timeout_time,
                   int poll_only_thread);
void erts_init_check_io(int *argc, char **argv);
void erts_check_io_interrupt(struct erts_poll_thread *pt, int set);
struct erts_poll_thread* erts_create_pollset_thread(int no, ErtsThrPrgrData *tpd);
#ifdef ERTS_ENABLE_LOCK_COUNT
void erts_lcnt_update_cio_locks(int enable);
#endif
typedef struct {
    ErtsPortTaskHandle task;
    ErtsSysFdType fd;
} ErtsIoTask;
ERTS_GLB_INLINE int erts_sched_poll_enabled(void);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE int erts_sched_poll_enabled(void)
{
#if ERTS_POLL_USE_SCHEDULER_POLLING
    extern ErtsPollSet *sched_pollset;
    return (sched_pollset != NULL);
#else
    return 0;
#endif
}
#endif
#endif
#if !defined(ERL_CHECK_IO_C__) && !defined(ERTS_ALLOC_C__)
#define ERL_CHECK_IO_INTERNAL__
#endif
#define ERTS_CHECK_IO_DRV_EV_STATE_LOCK_CNT 128
extern int erts_no_pollsets;
extern int erts_no_poll_threads;
#ifndef ERL_CHECK_IO_INTERNAL__
#define ERL_CHECK_IO_INTERNAL__
#include "erl_poll.h"
#include "erl_port_task.h"
typedef struct {
    Eterm inport;
    Eterm outport;
    ErtsIoTask iniotask;
    ErtsIoTask outiotask;
} ErtsDrvSelectDataState;
struct erts_nif_select_event {
    Eterm pid;
    ErtsMessage *mp;
};
typedef struct {
    struct erts_nif_select_event in;
    struct erts_nif_select_event out;
    struct erts_nif_select_event err;
} ErtsNifSelectDataState;
#endif