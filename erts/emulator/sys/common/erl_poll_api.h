void ERTS_POLL_EXPORT(erts_poll_init)(int *concurrent_waiters);
ErtsPollSet *ERTS_POLL_EXPORT(erts_poll_create_pollset)(int id);
ErtsPollEvents ERTS_POLL_EXPORT(erts_poll_control)(ErtsPollSet *ps,
                                                   ErtsSysFdType fd,
                                                   ErtsPollOp op,
                                                   ErtsPollEvents evts,
                                                   int *wake_poller);
int ERTS_POLL_EXPORT(erts_poll_wait)(ErtsPollSet *ps,
                                     ErtsPollResFd res[],
                                     int *length,
                                     ErtsThrPrgrData *tpd,
                                     ErtsMonotonicTime timeout_time);
void ERTS_POLL_EXPORT(erts_poll_interrupt)(ErtsPollSet *ps, int set);
int ERTS_POLL_EXPORT(erts_poll_max_fds)(void);
void ERTS_POLL_EXPORT(erts_poll_info)(ErtsPollSet *ps,
                                      ErtsPollInfo *info);
void ERTS_POLL_EXPORT(erts_poll_get_selected_events)(ErtsPollSet *ps,
                                                     ErtsPollEvents evts[],
                                                     int length);
#ifdef ERTS_ENABLE_LOCK_COUNT
void ERTS_POLL_EXPORT(erts_lcnt_enable_pollset_lock_count)(ErtsPollSet *, int enable);
#endif