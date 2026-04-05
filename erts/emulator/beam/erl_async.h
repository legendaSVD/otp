#ifndef ERL_ASYNC_H__
#define ERL_ASYNC_H__
#define ERTS_MAX_NO_OF_ASYNC_THREADS 1024
extern int erts_async_max_threads;
#define ERTS_ASYNC_THREAD_MIN_STACK_SIZE 16
#define ERTS_ASYNC_THREAD_MAX_STACK_SIZE 8192
extern int erts_async_thread_suggested_stack_size;
int erts_check_async_ready(void *);
int erts_async_ready_clean(void *, void *);
void *erts_get_async_ready_queue(Uint sched_id);
#define ERTS_ASYNC_READY_CLEAN 0
#define ERTS_ASYNC_READY_DIRTY 1
#define ERTS_ASYNC_READY_NEED_THR_PRGR 2
void erts_init_async(void);
void erts_exit_flush_async(void);
#endif