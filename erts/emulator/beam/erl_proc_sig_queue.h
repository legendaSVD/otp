#ifndef ERTS_PROC_SIG_QUEUE_H_TYPE__
#define ERTS_PROC_SIG_QUEUE_H_TYPE__
#if 0
#  define ERTS_PROC_SIG_HARD_DEBUG
#endif
#if 0
#  define ERTS_PROC_SIG_HARD_DEBUG_SIGQ_MSG_LEN
#endif
#if 0
#  define ERTS_PROC_SIG_HARD_DEBUG_RECV_MARKER
#endif
#if 0
#  define ERTS_PROC_SIG_HARD_DEBUG_SIGQ_BUFFERS
#endif
#define ERTS_HDBG_PRIVQ_LEN__(P)                                        \
    do {                                                                \
        Sint len = 0;                                                   \
        ErtsMessage *sig;                                               \
        ERTS_LC_ASSERT(erts_proc_lc_my_proc_locks((P))                  \
                       & ERTS_PROC_LOCK_MAIN);                          \
        for (sig = (P)->sig_qs.first; sig; sig = sig->next) {           \
            if (ERTS_SIG_IS_MSG(sig))                                   \
                len++;                                                  \
        }                                                               \
        ERTS_ASSERT((P)->sig_qs.mq_len == len);                         \
        for (sig = (P)->sig_qs.cont, len = 0; sig; sig = sig->next) {   \
            if (ERTS_SIG_IS_MSG(sig)) {                                 \
                len++;                                                  \
            }                                                           \
            else {                                                      \
                ErtsNonMsgSignal *nmsig = (ErtsNonMsgSignal *) sig;     \
                ERTS_ASSERT(nmsig->mlenoffs == len);                    \
                len = 0;                                                \
            }                                                           \
        }                                                               \
        ERTS_ASSERT((P)->sig_qs.mlenoffs == len);                       \
    } while (0)
#define ERTS_HDBG_INQ_LEN__(Q)                                          \
    do {                                                                \
        Sint len = 0;                                                   \
        ErtsMessage *sig;                                               \
        for (sig = (Q)->first; sig; sig = sig->next) {                  \
            if (ERTS_SIG_IS_MSG(sig)) {                                 \
                len++;                                                  \
            }                                                           \
            else {                                                      \
                ErtsNonMsgSignal *nmsig = (ErtsNonMsgSignal *) sig;     \
                ERTS_ASSERT(nmsig->mlenoffs == len);                    \
                len = 0;                                                \
            }                                                           \
        }                                                               \
        ERTS_ASSERT((Q)->mlenoffs == len);                              \
    } while (0)
#ifdef ERTS_PROC_SIG_HARD_DEBUG_SIGQ_MSG_LEN
#define ERTS_HDBG_PRIVQ_LEN(P) ERTS_HDBG_PRIVQ_LEN__((P))
#define ERTS_HDBG_INQ_LEN(Q) ERTS_HDBG_INQ_LEN((Q))
#else
#define ERTS_HDBG_PRIVQ_LEN(P)
#define ERTS_HDBG_INQ_LEN(Q)
#endif
struct erl_mesg;
struct erl_dist_external;
#define ERTS_SIGNAL_COMMON_FIELDS__     \
    struct erl_mesg *next;              \
    union {                             \
        struct erl_mesg **next;         \
        void *attachment;               \
    } specific;                         \
    Eterm tag
typedef struct {
    ERTS_SIGNAL_COMMON_FIELDS__;
} ErtsSignalCommon;
typedef struct {
    ERTS_SIGNAL_COMMON_FIELDS__;
    Sint mlenoffs;
} ErtsNonMsgSignal;
#define ERTS_SIG_Q_OP_MAX 20
#define ERTS_SIG_Q_OP_EXIT                      0
#define ERTS_SIG_Q_OP_EXIT_LINKED               1
#define ERTS_SIG_Q_OP_MONITOR_DOWN              2
#define ERTS_SIG_Q_OP_MONITOR                   3
#define ERTS_SIG_Q_OP_DEMONITOR                 4
#define ERTS_SIG_Q_OP_LINK                      5
#define ERTS_SIG_Q_OP_UNLINK                    6
#define ERTS_SIG_Q_OP_GROUP_LEADER              7
#define ERTS_SIG_Q_OP_TRACE_CHANGE_STATE        8
#define ERTS_SIG_Q_OP_PERSISTENT_MON_MSG        9
#define ERTS_SIG_Q_OP_IS_ALIVE                  10
#define ERTS_SIG_Q_OP_PROCESS_INFO              11
#define ERTS_SIG_Q_OP_SYNC_SUSPEND              12
#define ERTS_SIG_Q_OP_RPC                       13
#define ERTS_SIG_Q_OP_DIST_SPAWN_REPLY          14
#define ERTS_SIG_Q_OP_ALTACT_MSG                15
#define ERTS_SIG_Q_OP_RECV_MARK                 16
#define ERTS_SIG_Q_OP_UNLINK_ACK                17
#define ERTS_SIG_Q_OP_ADJ_MSGQ                  18
#define ERTS_SIG_Q_OP_FLUSH			19
#define ERTS_SIG_Q_OP_NIF_SELECT		ERTS_SIG_Q_OP_MAX
#define ERTS_SIG_Q_TYPE_MAX (ERTS_MON_LNK_TYPE_MAX + 11)
#define ERTS_SIG_Q_TYPE_UNDEFINED \
    (ERTS_MON_LNK_TYPE_MAX + 1)
#define ERTS_SIG_Q_TYPE_DIST_LINK \
    (ERTS_MON_LNK_TYPE_MAX + 2)
#define ERTS_SIG_Q_TYPE_GEN_EXIT \
    (ERTS_MON_LNK_TYPE_MAX + 3)
#define ERTS_SIG_Q_TYPE_DIST_PROC_DEMONITOR \
    (ERTS_MON_LNK_TYPE_MAX + 4)
#define ERTS_SIG_Q_TYPE_ADJUST_TRACE_INFO \
    (ERTS_MON_LNK_TYPE_MAX + 5)
#define ERTS_SIG_Q_TYPE_DIST \
    (ERTS_MON_LNK_TYPE_MAX + 6)
#define ERTS_SIG_Q_TYPE_DIST_FRAG \
    (ERTS_MON_LNK_TYPE_MAX + 7)
#define ERTS_SIG_Q_TYPE_HEAP \
    (ERTS_MON_LNK_TYPE_MAX + 8)
#define ERTS_SIG_Q_TYPE_OFF_HEAP \
    (ERTS_MON_LNK_TYPE_MAX + 9)
#define ERTS_SIG_Q_TYPE_HEAP_FRAG \
    (ERTS_MON_LNK_TYPE_MAX + 10)
#define ERTS_SIG_Q_TYPE_CLA \
    ERTS_SIG_Q_TYPE_MAX
#define ERTS_SIG_IS_DIST_ALTACT_MSG_TAG(Tag)                            \
    ((((Tag) & (ERTS_PROC_SIG_TYPE_MASK                                 \
                | ERTS_PROC_SIG_OP_MASK                                 \
                | ERTS_PROC_SIG_BASE_TAG_MASK))                         \
      == ERTS_PROC_SIG_MAKE_TAG(ERTS_SIG_Q_OP_ALTACT_MSG,               \
                                ERTS_SIG_Q_TYPE_DIST,                   \
                                0))                                     \
     | (((Tag) & (ERTS_PROC_SIG_TYPE_MASK                               \
                  | ERTS_PROC_SIG_OP_MASK                               \
                  | ERTS_PROC_SIG_BASE_TAG_MASK))                       \
        == ERTS_PROC_SIG_MAKE_TAG(ERTS_SIG_Q_OP_ALTACT_MSG,             \
                                  ERTS_SIG_Q_TYPE_DIST_FRAG,            \
                                  0)))
#define ERTS_SIG_IS_DIST_ALTACT_MSG(sig)                                \
    ERTS_SIG_IS_DIST_ALTACT_MSG_TAG(((ErtsSignal *) (sig))->common.tag)
#define ERTS_SIG_IS_OFF_HEAP_ALTACT_MSG_TAG(Tag)                      \
    (((Tag) & (ERTS_PROC_SIG_TYPE_MASK                               \
               | ERTS_PROC_SIG_OP_MASK                               \
               | ERTS_PROC_SIG_BASE_TAG_MASK))                       \
     == ERTS_PROC_SIG_MAKE_TAG(ERTS_SIG_Q_OP_ALTACT_MSG,              \
                               ERTS_SIG_Q_TYPE_OFF_HEAP,             \
                               0))
#define ERTS_SIG_IS_OFF_HEAP_ALTACT_MSG(sig)                          \
    ERTS_SIG_IS_OFF_HEAP_ALTACT_MSG_TAG(((ErtsSignal *) (sig))->common.tag)
#define ERTS_SIG_IS_HEAP_ALTACT_MSG_TAG(Tag)                          \
    (((Tag) & (ERTS_PROC_SIG_TYPE_MASK                               \
               | ERTS_PROC_SIG_OP_MASK                               \
               | ERTS_PROC_SIG_BASE_TAG_MASK))                       \
     == ERTS_PROC_SIG_MAKE_TAG(ERTS_SIG_Q_OP_ALTACT_MSG,              \
                               ERTS_SIG_Q_TYPE_HEAP,                 \
                               0))
#define ERTS_SIG_IS_HEAP_ALTACT_MSG(sig)                              \
    ERTS_SIG_IS_HEAP_ALTACT_MSG_TAG(((ErtsSignal *) (sig))->common.tag)
#define ERTS_SIG_IS_HEAP_FRAG_ALTACT_MSG_TAG(Tag)                     \
    (((Tag) & (ERTS_PROC_SIG_TYPE_MASK                               \
               | ERTS_PROC_SIG_OP_MASK                               \
               | ERTS_PROC_SIG_BASE_TAG_MASK))                       \
     == ERTS_PROC_SIG_MAKE_TAG(ERTS_SIG_Q_OP_ALTACT_MSG,              \
                               ERTS_SIG_Q_TYPE_HEAP_FRAG,            \
                               0))
#define ERTS_SIG_IS_HEAP_FRAG_ALTACT_MSG(sig)                         \
    ERTS_SIG_IS_HEAP_FRAG_ALTACT_MSG_TAG(((ErtsSignal *) (sig))->common.tag)
#define ERTS_RECV_MARKER_TAG                                         \
    (ERTS_PROC_SIG_MAKE_TAG(ERTS_SIG_Q_OP_RECV_MARK,		     \
                            ERTS_SIG_Q_TYPE_UNDEFINED, 0))
#define ERTS_SIG_IS_RECV_MARKER(Sig)                                 \
    (((ErtsSignal *) (Sig))->common.tag == ERTS_RECV_MARKER_TAG)
#define ERTS_RECV_MARKER_PASS_MAX 4
typedef struct {
    ErtsNonMsgSignal common;
    Eterm from;
    Uint64 id;
} ErtsSigUnlinkOp;
#define ERTS_SIG_HANDLE_REDS_MAX_PREFERED (CONTEXT_REDS/40)
#ifdef ERTS_PROC_SIG_HARD_DEBUG
#  define ERTS_HDBG_CHECK_SIGNAL_IN_QUEUE(P, B) \
    ERTS_HDBG_CHECK_SIGNAL_IN_QUEUE__((P), (B), "")
#  define ERTS_HDBG_CHECK_SIGNAL_PRIV_QUEUE(P, QL) \
    ERTS_HDBG_CHECK_SIGNAL_PRIV_QUEUE__((P), (QL), "")
#  define ERTS_HDBG_CHECK_SIGNAL_IN_QUEUE__(P, B , What) \
    erts_proc_sig_hdbg_check_in_queue((P), (B), (What), __FILE__, __LINE__)
#  define ERTS_HDBG_CHECK_SIGNAL_PRIV_QUEUE__(P, QL, What)              \
    erts_proc_sig_hdbg_check_priv_queue((P), (QL), (What), __FILE__, __LINE__)
struct process;
void erts_proc_sig_hdbg_check_priv_queue(struct process *c_p, int qlock,
                                         char *what, char *file, int line);
struct ErtsSignalInQueue_;
void erts_proc_sig_hdbg_check_in_queue(struct process *c_p,
                                       struct ErtsSignalInQueue_ *buffer,
                                       char *what, char *file, int line);
#else
#  define ERTS_HDBG_CHECK_SIGNAL_IN_QUEUE(P, B)
#  define ERTS_HDBG_CHECK_SIGNAL_PRIV_QUEUE(P, QL)
#  define ERTS_HDBG_CHECK_SIGNAL_IN_QUEUE__(P, B, What)
#define ERTS_HDBG_CHECK_SIGNAL_PRIV_QUEUE__(P, QL, What)
#endif
#ifdef ERTS_PROC_SIG_HARD_DEBUG_RECV_MARKER
#define ERTS_HDBG_CHK_RECV_MRKS(P) \
    erl_proc_sig_hdbg_chk_recv_marker_block((P))
struct process;
void erl_proc_sig_hdbg_chk_recv_marker_block(struct process *c_p);
#else
#define ERTS_HDBG_CHK_RECV_MRKS(P)
#endif
#endif
#if !defined(ERTS_PROC_SIG_QUEUE_H__) && !defined(ERTS_PROC_SIG_QUEUE_TYPE_ONLY)
#define ERTS_PROC_SIG_QUEUE_H__
#include "erl_process.h"
#include "erl_bif_unique.h"
void erts_proc_sig_queue_maybe_install_buffers(Process* p, erts_aint32_t state);
void erts_proc_sig_queue_flush_and_deinstall_buffers(Process* proc);
void erts_proc_sig_queue_flush_buffers(Process* proc);
void erts_proc_sig_queue_lock(Process* proc);
ERTS_GLB_INLINE ErtsSignalInQueueBufferArray*
erts_proc_sig_queue_get_buffers(Process* p, int *need_unread);
ERTS_GLB_INLINE void
erts_proc_sig_queue_unget_buffers(ErtsSignalInQueueBufferArray* buffers,
                                  int need_unget);
int erts_proc_sig_queue_try_enqueue_to_buffer(Eterm from,
                                              Process* receiver,
                                              ErtsProcLocks receiver_locks,
                                              ErtsMessage* first,
                                              ErtsMessage** last,
                                              ErtsMessage** last_next,
                                              Uint len);
int erts_proc_sig_queue_force_buffers(Process*);
#define ERTS_SIG_Q_OP_BITS      8
#define ERTS_SIG_Q_OP_SHIFT     0
#define ERTS_SIG_Q_OP_MASK      ((1 << ERTS_SIG_Q_OP_BITS) - 1)
#define ERTS_SIG_Q_TYPE_BITS    8
#define ERTS_SIG_Q_TYPE_SHIFT   ERTS_SIG_Q_OP_BITS
#define ERTS_SIG_Q_TYPE_MASK    ((1 << ERTS_SIG_Q_TYPE_BITS) - 1)
#define ERTS_SIG_Q_NON_X_BITS__ (_HEADER_ARITY_OFFS \
                                 + ERTS_SIG_Q_OP_BITS \
                                 + ERTS_SIG_Q_TYPE_BITS)
#define ERTS_SIG_Q_XTRA_BITS    (32 - ERTS_SIG_Q_NON_X_BITS__)
#define ERTS_SIG_Q_XTRA_SHIFT   (ERTS_SIG_Q_OP_BITS \
                                 + ERTS_SIG_Q_TYPE_BITS)
#define ERTS_SIG_Q_XTRA_MASK    ((1 << ERTS_SIG_Q_XTRA_BITS) - 1)
#define ERTS_PROC_SIG_OP(Tag) \
    ((int) (_unchecked_thing_arityval((Tag)) \
            >> ERTS_SIG_Q_OP_SHIFT) & ERTS_SIG_Q_OP_MASK)
#define ERTS_PROC_SIG_TYPE(Tag) \
    ((Uint16) (_unchecked_thing_arityval((Tag)) \
               >> ERTS_SIG_Q_TYPE_SHIFT) & ERTS_SIG_Q_TYPE_MASK)
#define ERTS_PROC_SIG_XTRA(Tag) \
    ((Uint32) (_unchecked_thing_arityval((Tag)) \
               >> ERTS_SIG_Q_XTRA_SHIFT) & ERTS_SIG_Q_XTRA_MASK)
#define ERTS_PROC_SIG_MAKE_TAG(Op, Type, Xtra)                  \
    (ASSERT(0 <= (Xtra) && (Xtra) <= ERTS_SIG_Q_XTRA_MASK),     \
     _make_header((((Type) & ERTS_SIG_Q_TYPE_MASK)              \
                   << ERTS_SIG_Q_TYPE_SHIFT)                    \
                  | (((Op) & ERTS_SIG_Q_OP_MASK)                \
                     << ERTS_SIG_Q_OP_SHIFT)                    \
                  | (((Xtra) & ERTS_SIG_Q_XTRA_MASK)            \
                     << ERTS_SIG_Q_XTRA_SHIFT),                 \
                  _TAG_HEADER_EXTERNAL_PID))
#define ERTS_PROC_SIG_BASE_TAG_MASK \
    ((1 << _HEADER_ARITY_OFFS)-1)
#define ERTS_PROC_SIG_OP_MASK \
    (ERTS_SIG_Q_OP_MASK << (ERTS_SIG_Q_OP_SHIFT + _HEADER_ARITY_OFFS))
#define ERTS_PROC_SIG_TYPE_MASK \
    (ERTS_SIG_Q_TYPE_MASK << (ERTS_SIG_Q_TYPE_SHIFT + _HEADER_ARITY_OFFS))
#define ERTS_PROC_SIG_XTRA_MASK \
    (ERTS_SIG_Q_XTRA_MASK << (ERTS_SIG_Q_XTRA_SHIFT + _HEADER_ARITY_OFFS))
struct dist_entry_;
#define ERTS_PROC_HAS_INCOMING_SIGNALS(P)                               \
    (!!(erts_atomic32_read_nob(&(P)->state)                             \
        & (ERTS_PSFLG_SIG_Q                                             \
           | ERTS_PSFLG_NMSG_SIG_IN_Q                                   \
           | ERTS_PSFLG_MSG_SIG_IN_Q)))
void
erts_proc_sig_send_exit(ErtsPTabElementCommon *sender, Eterm from, Eterm to,
                        Eterm reason, Eterm token, int normal_kills, int prio);
void
erts_proc_sig_send_dist_exit(DistEntry *dep,
                             Eterm from, Eterm to,
                             ErtsDistExternal *dist_ext,
                             ErlHeapFragment *hfrag,
                             Eterm reason, Eterm token, int prio);
void
erts_proc_sig_send_link_exit_noconnection(ErtsLink *lnk);
void
erts_proc_sig_send_link_exit(ErtsPTabElementCommon *sender, Eterm from,
                             ErtsLink *lnk, Eterm reason, Eterm token);
int
erts_proc_sig_send_link(ErtsPTabElementCommon *sender, Eterm from,
                        Eterm to, ErtsLink *lnk);
ERTS_GLB_INLINE
Uint64 erts_proc_sig_new_unlink_id(ErtsPTabElementCommon *sender);
ErtsSigUnlinkOp *
erts_proc_sig_make_unlink_op(ErtsPTabElementCommon *sender, Eterm from);
void
erts_proc_sig_destroy_unlink_op(ErtsSigUnlinkOp *sulnk);
Uint64
erts_proc_sig_send_unlink(ErtsPTabElementCommon *sender, Eterm from,
                          ErtsLink *lnk);
void
erts_proc_sig_send_unlink_ack(ErtsPTabElementCommon *sender, Eterm from,
                              ErtsSigUnlinkOp *sulnk);
void
erts_proc_sig_send_dist_link_exit(struct dist_entry_ *dep,
                                  Eterm from, Eterm to,
                                  ErtsDistExternal *dist_ext,
                                  ErlHeapFragment *hfrag,
                                  Eterm reason, Eterm token);
void
erts_proc_sig_send_dist_unlink(DistEntry *dep, Uint32 conn_id,
                               Eterm from, Eterm to, Uint64 id);
void
erts_proc_sig_send_dist_unlink_ack(DistEntry *dep,
                                   Uint32 conn_id, Eterm from, Eterm to,
                                   Uint64 id);
void
erts_proc_sig_send_monitor_down(ErtsPTabElementCommon *sender, Eterm from,
                                ErtsMonitor *mon, Eterm reason);
void
erts_proc_sig_send_demonitor(ErtsPTabElementCommon *sender, Eterm from,
                             int system, ErtsMonitor *mon);
int
erts_proc_sig_send_monitor(ErtsPTabElementCommon *sender, Eterm from,
                           ErtsMonitor *mon, Eterm to);
void
erts_proc_sig_send_dist_monitor_down(DistEntry *dep, Eterm ref,
                                     Eterm from, Eterm to,
                                     ErtsDistExternal *dist_ext,
                                     ErlHeapFragment *hfrag,
                                     Eterm reason);
void
erts_proc_sig_send_dist_demonitor(Eterm from, Eterm to, Eterm ref);
void
erts_proc_sig_send_monitor_nodes_msg(Eterm key, Eterm to,
                                     Eterm msg, Uint msg_sz);
void
erts_proc_sig_send_monitor_time_offset_msg(Eterm key, Eterm to,
                                           Eterm msg, Uint msg_sz);
void
erts_proc_sig_send_group_leader(Process *c_p, Eterm to, Eterm gl,
                                Eterm ref);
int
erts_proc_sig_send_is_alive_request(Process *c_p, Eterm to,
                                    Eterm ref);
int
erts_proc_sig_send_process_info_request(Process *c_p,
                                        Eterm to,
                                        int *item_ix,
                                        Eterm *item_extra,
                                        int len,
                                        int flags,
                                        Uint reserve_size,
                                        Eterm ref);
void
erts_proc_sig_send_sync_suspend(Process *c_p, Eterm to,
                                Eterm tag, Eterm reply);
Eterm
erts_proc_sig_send_rpc_request(Process *c_p,
                               Eterm to,
                               int reply,
                               Eterm (*func)(Process *, void *, int *, ErlHeapFragment **),
                               void *arg);
Eterm
erts_proc_sig_send_rpc_request_prio(Process *c_p,
                                    Eterm to,
                                    int reply,
                                    Eterm (*func)(Process *, void *, int *, ErlHeapFragment **),
                                    void *arg,
                                    int prio);
int
erts_proc_sig_send_dist_spawn_reply(Eterm node,
                                    Eterm ref,
                                    Eterm to,
                                    ErtsLink *lnk,
                                    Eterm result,
                                    Eterm token);
void
erts_proc_sig_send_cla_request(Process *c_p, Eterm to, Eterm req_id);
void
erts_proc_sig_send_move_msgq_off_heap(Eterm to);
int
erts_proc_sig_send_nif_select(Eterm to, ErtsMessage *msg);
int
erts_proc_sig_handle_incoming(Process *c_p, erts_aint32_t *statep,
                              int *redsp, int max_reds,
                              int local_only);
int
erts_proc_sig_handle_exit(Process *c_p, Sint *redsp,
                          ErtsProcExitContext *pe_ctxt_p);
int
erts_proc_sig_receive_helper(Process *c_p, int fcalls,
                             int neg_o_reds, ErtsMessage **msgpp,
                             int *get_outp);
#define ERTS_PROC_SIG_FLUSH_FLG_FROM_ALL            (1 << 1)
#define ERTS_PROC_SIG_FLUSH_FLG_FROM_ID             (1 << 2)
#define ERTS_PROC_SIG_FLUSH_FLGS                                \
    (ERTS_PROC_SIG_FLUSH_FLG_FROM_ALL                           \
     | ERTS_PROC_SIG_FLUSH_FLG_FROM_ID)
void
erts_proc_sig_init_flush_signals(Process *c_p, int flags, Eterm from);
ERTS_GLB_INLINE Sint erts_proc_sig_fetch(Process *p);
ERTS_GLB_INLINE Sint
erts_proc_sig_privqs_len(Process *c_p, Sint max_sigs, Sint max_nmsigs);
erts_aint32_t
erts_enqueue_signals(Process *rp, ErtsMessage *first,
                     ErtsMessage **last,
                     Uint msg_cnt,
                     erts_aint32_t in_state);
void
erts_proc_sig_send_pending(Process *c_p, ErtsSchedulerData* esdp);
void
erts_proc_sig_send_altact_msg(Process *c_p, Eterm from, Eterm to,
                              Eterm msg, Eterm token, int prio);
void
erts_proc_sig_send_dist_altact_msg(Eterm from, Eterm alias,
                                   ErtsDistExternal *edep,
                                   ErlHeapFragment *hfrag, Eterm token,
                                   int prio);
ERTS_GLB_INLINE void erts_proc_notify_new_sig(Process* rp, erts_aint32_t state,
                                              erts_aint32_t enable_flag);
ERTS_GLB_INLINE void erts_proc_notify_new_message(Process *p,
                                                  ErtsProcLocks locks);
void
erts_ensure_dirty_proc_signals_handled(Process *proc,
                                       erts_aint32_t state,
                                       erts_aint32_t prio,
                                       ErtsProcLocks locks);
typedef struct {
    Uint size;
    ErtsMessage *msgp;
} ErtsMessageInfo;
Uint erts_proc_sig_prep_msgq_for_inspection(Process *c_p,
                                            Process *rp,
                                            ErtsProcLocks rp_locks,
                                            int info_on_self,
                                            ErtsMessageInfo *mip,
                                            Sint *msgq_lenp);
void erts_proc_sig_move_msgs_to_heap(Process *c_p);
Uint erts_proc_sig_signal_size(ErtsSignal *sig);
void
erts_proc_sig_clear_seq_trace_tokens(Process *c_p);
void
erts_proc_sig_handle_pending_suspend(Process *c_p);
int
erts_proc_sig_decode_dist(Process *proc, ErtsProcLocks proc_locks,
                          ErtsMessage *msgp, int force_off_heap);
ERTS_GLB_INLINE erts_aint32_t
erts_proc_sig_check_wait_dirty_handle_signals(Process *c_p,
                                              erts_aint32_t state_in);
void erts_proc_sig_do_wait_dirty_handle_signals__(Process *c_p);
ErtsDistExternal *
erts_proc_sig_get_external(ErtsMessage *msgp);
void
erts_proc_sig_cleanup_non_msg_signal(ErtsMessage *sig);
ERTS_GLB_INLINE Eterm erts_msgq_recv_marker_insert(Process *c_p);
ERTS_GLB_INLINE void erts_msgq_recv_marker_bind(Process *c_p,
						Eterm insert_id,
						Eterm bind_id);
ERTS_GLB_INLINE void erts_msgq_recv_marker_insert_bind(Process *c_p,
						       Eterm id);
ERTS_GLB_INLINE void erts_msgq_recv_marker_set_save(Process *c_p, Eterm id);
ERTS_GLB_INLINE void erts_msgq_recv_marker_clear(Process *c_p, Eterm id);
ERTS_GLB_INLINE ErtsMessage *erts_msgq_peek_msg(Process *c_p);
ERTS_GLB_INLINE void erts_msgq_unlink_msg(Process *c_p,
					  ErtsMessage *msgp);
ERTS_GLB_INLINE void erts_msgq_set_save_first(Process *c_p);
ERTS_GLB_INLINE void erts_msgq_unlink_msg_set_save_first(Process *c_p,
                                                         ErtsMessage *msgp);
ERTS_GLB_INLINE void erts_msgq_set_save_next(Process *c_p);
ERTS_GLB_INLINE void erts_msgq_set_save_end(Process *c_p);
void erts_proc_sig_cleanup_queues(Process *c_p);
typedef enum {
    ERTS_PRIO_ITEM_TYPE_ALIAS,
    ERTS_PRIO_ITEM_TYPE_LINK,
    ERTS_PRIO_ITEM_TYPE_MONITOR,
} ErtsPrioItemType;
void erts_proc_sig_prio_item_deleted(Process *c_p, ErtsPrioItemType type);
void erts_proc_sig_prio_item_added(Process *c_p, ErtsPrioItemType type);
void erts_proc_sig_queue_init(void);
void
erts_proc_sig_debug_foreach_sig(Process *c_p,
                                void (*msg_func)(ErtsMessage *, void *),
                                void (*oh_func)(ErlOffHeap *, void *),
                                ErtsMonitorFunc mon_func,
                                ErtsLinkFunc lnk_func,
                                void (*ext_func)(ErtsDistExternal *, void *),
                                void *arg);
extern Process *erts_dirty_process_signal_handler;
extern Process *erts_dirty_process_signal_handler_high;
extern Process *erts_dirty_process_signal_handler_max;
void erts_proc_sig_fetch__(Process *proc,
                           ErtsSignalInQueueBufferArray* buffers,
                           int need_unget_buffers);
ERTS_GLB_INLINE void erts_chk_sys_mon_long_msgq_on(Process *proc);
ERTS_GLB_INLINE void erts_chk_sys_mon_long_msgq_off(Process *proc);
ERTS_GLB_INLINE int erts_msgq_eq_recv_mark_id__(Eterm term1, Eterm term2);
ERTS_GLB_INLINE ErtsMessage **
erts_msgq_recv_marker_pending_set_save__(Process *c_p,
                                         ErtsRecvMarkerBlock *blkp,
                                         ErtsRecvMarker *markp,
                                         int ix);
ERTS_GLB_INLINE void
erts_msgq_recv_marker_set_save__(Process *c_p,
                                 ErtsRecvMarkerBlock *blkp,
                                 ErtsRecvMarker *markp,
                                 int ix);
Eterm erts_msgq_recv_marker_create_insert(Process *c_p, Eterm id);
void erts_msgq_recv_marker_create_insert_set_save(Process *c_p, Eterm id);
ErtsMessage **erts_msgq_pass_recv_markers(Process *c_p,
					  ErtsMessage **markpp);
void erts_msgq_remove_leading_recv_markers_set_save_first(Process *c_p);
#define ERTS_RECV_MARKER_IX__(BLKP, MRKP) \
    ((int) ((MRKP) - &(BLKP)->marker[0]))
#define ERTS_PROC_SIG_RECV_MARK_CLEAR_PENDING_SET_SAVE__(BLKP) 		\
    do {								\
	if ((BLKP)->pending_set_save_ix >= 0) {				\
	    int clr_ix__ = (BLKP)->pending_set_save_ix;			\
	    ErtsRecvMarker *clr_markp__ = &(BLKP)->marker[clr_ix__];	\
	    ASSERT(!clr_markp__->in_msgq);				\
	    ASSERT(clr_markp__->in_sigq);				\
	    ASSERT(clr_markp__->set_save);				\
	    clr_markp__->set_save = 0;					\
	    (BLKP)->pending_set_save_ix = -1;				\
	}								\
    } while (0)
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE Uint64
erts_proc_sig_new_unlink_id(ErtsPTabElementCommon *sender)
{
    Uint64 id;
    ASSERT(sender);
    if (is_internal_pid(sender->id)) {
        Process *c_p = ErtsContainerStruct(sender, Process, common);
        id = (Uint64) c_p->uniq++;
        if (id == 0) {
            id = (Uint64) c_p->uniq++;
        }
    } else {
        ASSERT(is_internal_port(sender->id));
        id = (Uint64) erts_raw_get_unique_monotonic_integer();
        if (id == 0) {
            id = (Uint64) erts_raw_get_unique_monotonic_integer();
        }
    }
    ASSERT(id != 0);
    return id;
}
ERTS_GLB_INLINE ErtsSignalInQueueBufferArray*
erts_proc_sig_queue_get_buffers(Process* p, int *need_unread)
{
    ErtsThrPrgrDelayHandle dhndl = erts_thr_progress_unmanaged_delay();
    ErtsSignalInQueueBufferArray* buffers =
        (ErtsSignalInQueueBufferArray*)erts_atomic_read_acqb(&p->sig_inq_buffers);
    *need_unread = 0;
    if (ERTS_THR_PRGR_DHANDLE_MANAGED == dhndl) {
        erts_thr_progress_unmanaged_continue(dhndl);
        return buffers;
    }
    if (buffers == NULL) {
        erts_thr_progress_unmanaged_continue(dhndl);
        return NULL;
    }
    erts_refc_inc(&buffers->dirty_refc, 2);
    erts_thr_progress_unmanaged_continue(dhndl);
    *need_unread = 1;
    return buffers;
}
ERTS_GLB_INLINE void
erts_proc_sig_queue_unget_buffers(ErtsSignalInQueueBufferArray* buffers,
                                  int need_unget)
{
    if (!need_unget) {
        return;
    } else {
        int i;
        erts_aint_t refc = erts_refc_dectest(&buffers->dirty_refc, 0);
        if (refc != 0) {
            return;
        }
        ASSERT(!buffers->alive);
        for (i = 0; i < ERTS_PROC_SIG_INQ_BUFFERED_NR_OF_BUFFERS; i++) {
            ASSERT(!buffers->slots[i].b.alive);
            erts_mtx_destroy(&buffers->slots[i].b.lock);
        }
        erts_free(ERTS_ALC_T_SIGQ_BUFFERS, buffers);
    }
}
ERTS_GLB_INLINE void
erts_chk_sys_mon_long_msgq_on(Process *proc)
{
    if ((proc->sig_qs.flags & FS_MON_MSGQ_LEN_HIGH)
        && proc->sig_qs.mq_len >= (Sint)erts_system_monitor_long_msgq_on) {
        monitor_long_msgq_on(proc);
    }
}
ERTS_GLB_INLINE void
erts_chk_sys_mon_long_msgq_off(Process *proc)
{
    if ((proc->sig_qs.flags & FS_MON_MSGQ_LEN_LOW)
        && proc->sig_qs.mq_len <= erts_system_monitor_long_msgq_off) {
        monitor_long_msgq_off(proc);
    }
}
ERTS_GLB_INLINE Sint
erts_proc_sig_fetch(Process *proc)
{
    Sint res;
    ErtsSignalInQueueBufferArray* buffers;
    int need_unget_buffers;
    ERTS_LC_ASSERT((erts_proc_lc_my_proc_locks(proc)
                    & (ERTS_PROC_LOCK_MAIN
                       | ERTS_PROC_LOCK_MSGQ))
                   == (ERTS_PROC_LOCK_MAIN
                       | ERTS_PROC_LOCK_MSGQ));
    ASSERT(!(proc->sig_qs.flags & FS_FLUSHING_SIGS)
           || ERTS_PROC_IS_EXITING(proc)
           || ERTS_IS_CRASH_DUMPING);
    ASSERT(!(proc->sig_qs.flags & FS_HANDLING_SIGS));
    ERTS_HDBG_CHECK_SIGNAL_IN_QUEUE(proc, &proc->sig_inq);
    ERTS_HDBG_CHECK_SIGNAL_PRIV_QUEUE(proc, !0);
    proc->sig_qs.flags &= ~FS_NON_FETCH_CNT_MASK;
    buffers = erts_proc_sig_queue_get_buffers(proc, &need_unget_buffers);
    if (buffers || proc->sig_inq.first)
        erts_proc_sig_fetch__(proc, buffers, need_unget_buffers);
    res = proc->sig_qs.mq_len;
    ERTS_HDBG_CHECK_SIGNAL_PRIV_QUEUE(proc, !0);
    return res;
}
ERTS_GLB_INLINE Sint
erts_proc_sig_privqs_len(Process *c_p, Sint max_sigs, Sint max_nmsigs)
{
    Sint res = c_p->sig_qs.mq_len;
    int no_nmsigs = 0;
    ErtsMessage **nmsigpp;
    ERTS_HDBG_PRIVQ_LEN(c_p);
    if (max_sigs < 0)
        max_sigs = ERTS_SWORD_MAX;
    if (res > max_sigs)
        return -res;
    nmsigpp = c_p->sig_qs.nmsigs.next;
    if (max_nmsigs < 0)
        max_nmsigs = ERTS_SWORD_MAX;
    res += c_p->sig_qs.mlenoffs;
    while (nmsigpp) {
        ErtsNonMsgSignal *nmsigp = (ErtsNonMsgSignal *) *nmsigpp;
        ASSERT(nmsigp);
        res += nmsigp->mlenoffs + 1;
        if (res > max_sigs)
            return -res;
        if (++no_nmsigs > max_nmsigs)
            return -res;
        nmsigpp = nmsigp->specific.next;
    }
    return res;
}
ERTS_GLB_INLINE void
erts_proc_notify_new_sig(Process* rp, erts_aint32_t state,
                         erts_aint32_t enable_flag)
{
    if ((!(state & (ERTS_PSFLG_EXITING
                    | ERTS_PSFLG_ACTIVE_SYS))) | (~state & enable_flag)) {
        state = erts_proc_sys_schedule(rp, state, enable_flag);
    }
    if (ERTS_PROC_IN_DIRTY_STATE(state)) {
        erts_ensure_dirty_proc_signals_handled(rp, state, -1, 0);
    }
}
ERTS_GLB_INLINE void
erts_proc_notify_new_message(Process *p, ErtsProcLocks locks)
{
    erts_aint32_t state = erts_atomic32_read_nob(&p->state);
    if (!(state & ERTS_PSFLG_ACTIVE))
	erts_schedule_process(p, state, locks);
    if (ERTS_PROC_NEED_DIRTY_SIG_HANDLING(state)) {
        erts_ensure_dirty_proc_signals_handled(p, state, -1, locks);
    }
}
ERTS_GLB_INLINE int
erts_msgq_eq_recv_mark_id__(Eterm term1, Eterm term2)
{
    int ix, arity;
    Eterm *tp1, *tp2;
    ASSERT(term1 == am_free || term1 == am_undefined || term1 == NIL
	   || is_small(term1) || is_big(term1) || is_internal_ref(term1));
    ASSERT(term2 == am_free || term2 == am_undefined || term2 == NIL
	   || is_small(term2) || is_big(term2) || is_internal_ref(term2));
    if (term1 == term2)
	return !0;
    if (!is_boxed(term1) || !is_boxed(term2))
	return 0;
    tp1 = boxed_val(term1);
    tp2 = boxed_val(term2);
    if (*tp1 != *tp2)
	return 0;
    arity = (int) thing_arityval(*tp1);
    for (ix = 1; ix <= arity; ix++) {
	if (tp1[ix] != tp2[ix])
	    return 0;
    }
    return !0;
}
ERTS_GLB_INLINE ErtsMessage **
erts_msgq_recv_marker_pending_set_save__(Process *c_p,
                                         ErtsRecvMarkerBlock *blkp,
                                         ErtsRecvMarker *markp,
                                         int ix)
{
    c_p->sig_qs.save = c_p->sig_qs.last;
    ASSERT(!(*c_p->sig_qs.save));
    c_p->sig_qs.flags &= ~FS_PRIO_MQ_SAVE;
    markp->set_save = !0;
    ASSERT(blkp->pending_set_save_ix == -1);
    ASSERT(ix == ERTS_RECV_MARKER_IX__(blkp, markp));
    blkp->pending_set_save_ix = ix;
    return c_p->sig_qs.last;
}
ERTS_GLB_INLINE void
erts_msgq_recv_marker_set_save__(Process *c_p,
				 ErtsRecvMarkerBlock *blkp,
				 ErtsRecvMarker *markp,
				 int ix)
{
    ERTS_PROC_SIG_RECV_MARK_CLEAR_PENDING_SET_SAVE__(blkp);
    ASSERT(markp->proc == c_p);
    ASSERT(!markp->set_save);
    ASSERT(markp->in_sigq);
    if (!markp->in_msgq) {
        (void) erts_msgq_recv_marker_pending_set_save__(c_p, blkp,
                                                        markp, ix);
    }
    else {
        ErtsMessage **sigpp, *sigp;
        if (!markp->in_prioq) {
        empty_prioq:
            sigpp = &markp->sig.common.next;
            sigp = *sigpp;
            c_p->sig_qs.flags &= ~FS_PRIO_MQ_SAVE;
        }
        else {
            if (!(c_p->sig_qs.flags & FS_PRIO_MQ_END_MARK)) {
                markp->in_prioq = 0;
                goto empty_prioq;
            }
            sigpp = &c_p->sig_qs.first;
            sigp = *sigpp;
            ASSERT(sigp && c_p->sig_qs.flags & FS_PRIO_MQ);
            c_p->sig_qs.flags |= FS_PRIO_MQ_SAVE;
        }
	if (sigp && ERTS_SIG_IS_RECV_MARKER(sigp))
	    sigpp = erts_msgq_pass_recv_markers(c_p, sigpp);
        c_p->sig_qs.save = sigpp;
    }
    ERTS_MQ_SET_SAVE_INFO(c_p, FS_SET_SAVE_INFO_RCVM);
    blkp->set_save_ix = ix;
}
ERTS_GLB_INLINE void
erts_msgq_recv_marker_clear(Process *c_p, Eterm id)
{
    ErtsRecvMarkerBlock *blkp = c_p->sig_qs.recv_mrk_blk;
    int ix;
    ASSERT(!(c_p->sig_qs.flags & FS_HANDLING_SIGS));
    if (!is_small(id) && !is_big(id) && !is_internal_ref(id))
	return;
    if (!blkp)
	return;
    for (ix = 0; ix < ERTS_RECV_MARKER_BLOCK_SIZE; ix++) {
	if (erts_msgq_eq_recv_mark_id__(blkp->ref[ix], id)) {
	    blkp->unused++;
	    blkp->ref[ix] = am_undefined;
	    blkp->marker[ix].pass = ERTS_RECV_MARKER_PASS_MAX;
	    break;
	}
    }
}
ERTS_GLB_INLINE Eterm
erts_msgq_recv_marker_insert(Process *c_p)
{
    ASSERT(!(c_p->sig_qs.flags & FS_HANDLING_SIGS));
    erts_proc_sig_queue_lock(c_p);
    erts_proc_sig_fetch(c_p);
    erts_proc_unlock(c_p, ERTS_PROC_LOCK_MSGQ);
    if (c_p->sig_qs.cont || c_p->sig_qs.first)
	return erts_msgq_recv_marker_create_insert(c_p, am_new_uniq);
    return am_undefined;
}
ERTS_GLB_INLINE void erts_msgq_recv_marker_bind(Process *c_p,
						Eterm insert_id,
						Eterm bind_id)
{
    ASSERT(!(c_p->sig_qs.flags & FS_HANDLING_SIGS));
    if (is_small(insert_id) || is_big(insert_id)) {
	ErtsRecvMarkerBlock *blkp = c_p->sig_qs.recv_mrk_blk;
	if (blkp) {
	    int ix;
	    for (ix = 0; ix < ERTS_RECV_MARKER_BLOCK_SIZE; ix++) {
		if (erts_msgq_eq_recv_mark_id__(blkp->ref[ix], insert_id)) {
		    if (is_internal_ref(bind_id))
			blkp->ref[ix] = bind_id;
		    else {
			blkp->unused++;
			blkp->ref[ix] = am_undefined;
			blkp->marker[ix].pass = ERTS_RECV_MARKER_PASS_MAX;
		    }
		    break;
		}
	    }
	}
    }
}
ERTS_GLB_INLINE void
erts_msgq_recv_marker_insert_bind(Process *c_p, Eterm id)
{
    ASSERT(!(c_p->sig_qs.flags & FS_HANDLING_SIGS));
    if (is_internal_ref(id)) {
        erts_proc_sig_queue_lock(c_p);
	erts_proc_sig_fetch(c_p);
	erts_proc_unlock(c_p, ERTS_PROC_LOCK_MSGQ);
	if (c_p->sig_qs.cont || c_p->sig_qs.first)
	    (void) erts_msgq_recv_marker_create_insert(c_p, id);
    }
}
ERTS_GLB_INLINE void
erts_msgq_recv_marker_set_save(Process *c_p, Eterm id)
{
    ASSERT(!(c_p->sig_qs.flags & FS_HANDLING_SIGS));
    if (is_internal_ref(id)) {
	ErtsRecvMarkerBlock *blkp = c_p->sig_qs.recv_mrk_blk;
	if (blkp) {
	    int ix;
	    for (ix = 0; ix < ERTS_RECV_MARKER_BLOCK_SIZE; ix++) {
		if (erts_msgq_eq_recv_mark_id__(blkp->ref[ix], id)) {
		    ErtsRecvMarker *markp = &blkp->marker[ix];
		    erts_msgq_recv_marker_set_save__(c_p, blkp, markp, ix);
		    break;
		}
	    }
	}
    }
}
ERTS_GLB_INLINE ErtsMessage *
erts_msgq_peek_msg(Process *c_p)
{
    ASSERT(!(c_p->sig_qs.flags & FS_HANDLING_SIGS));
    ASSERT(!(*c_p->sig_qs.save) || ERTS_SIG_IS_MSG(*c_p->sig_qs.save));
    return *c_p->sig_qs.save;
}
ERTS_GLB_INLINE void
erts_msgq_unlink_msg(Process *c_p, ErtsMessage *msgp)
{
    ErtsMessage *sigp = msgp->next;
    ASSERT(!(c_p->sig_qs.flags & FS_HANDLING_SIGS));
    ERTS_HDBG_CHECK_SIGNAL_PRIV_QUEUE__(c_p, 0, "before");
    *c_p->sig_qs.save = sigp;
    c_p->sig_qs.mq_len--;
    ASSERT(c_p->sig_qs.mq_len >= 0);
    erts_chk_sys_mon_long_msgq_off(c_p);
    if (sigp && ERTS_SIG_IS_RECV_MARKER(sigp)) {
        ErtsMessage **sigpp = c_p->sig_qs.save;
        ((ErtsRecvMarker *) sigp)->prev_next = sigpp;
        c_p->sig_qs.save = erts_msgq_pass_recv_markers(c_p, sigpp);
	sigp = *c_p->sig_qs.save;
    }
    if (!sigp)
        c_p->sig_qs.last = c_p->sig_qs.save;
    ERTS_HDBG_CHECK_SIGNAL_PRIV_QUEUE__(c_p, 0, "after");
}
ERTS_GLB_INLINE void
erts_msgq_set_save_first(Process *c_p)
{
    ErtsRecvMarkerBlock *blkp = c_p->sig_qs.recv_mrk_blk;
    ASSERT(!(c_p->sig_qs.flags & FS_HANDLING_SIGS));
    if (blkp) {
	ERTS_PROC_SIG_RECV_MARK_CLEAR_PENDING_SET_SAVE__(blkp);
    }
    if (c_p->sig_qs.first && ERTS_SIG_IS_RECV_MARKER(c_p->sig_qs.first))
	erts_msgq_remove_leading_recv_markers_set_save_first(c_p);
    else {
        if (c_p->sig_qs.flags & FS_PRIO_MQ_END_MARK) {
            c_p->sig_qs.flags |= FS_PRIO_MQ_SAVE;
        }
        else {
            ASSERT(!(c_p->sig_qs.flags & FS_PRIO_MQ_SAVE));
        }
        ERTS_MQ_SET_SAVE_INFO(c_p, FS_SET_SAVE_INFO_FIRST);
        c_p->sig_qs.save = &c_p->sig_qs.first;
    }
    ASSERT(ERTS_MQ_GET_SAVE_INFO(c_p) == FS_SET_SAVE_INFO_FIRST);
}
ERTS_GLB_INLINE void
erts_msgq_unlink_msg_set_save_first(Process *c_p, ErtsMessage *msgp)
{
    ErtsMessage *sigp = msgp->next;
    ASSERT(!(c_p->sig_qs.flags & FS_HANDLING_SIGS));
    ERTS_HDBG_CHECK_SIGNAL_PRIV_QUEUE__(c_p, 0, "before");
    *c_p->sig_qs.save = sigp;
    c_p->sig_qs.mq_len--;
    ASSERT(c_p->sig_qs.mq_len >= 0);
    erts_chk_sys_mon_long_msgq_off(c_p);
    if (!sigp)
        c_p->sig_qs.last = c_p->sig_qs.save;
    else if (ERTS_SIG_IS_RECV_MARKER(sigp))
        ((ErtsRecvMarker *) sigp)->prev_next = c_p->sig_qs.save;
    erts_msgq_set_save_first(c_p);
    ERTS_HDBG_CHECK_SIGNAL_PRIV_QUEUE__(c_p, 0, "after");
}
ERTS_GLB_INLINE void
erts_msgq_set_save_next(Process *c_p)
{
    ErtsMessage *sigp = (*c_p->sig_qs.save)->next;
    ErtsMessage **sigpp = &(*c_p->sig_qs.save)->next;
    ASSERT(!(c_p->sig_qs.flags & FS_HANDLING_SIGS));
    ERTS_HDBG_CHECK_SIGNAL_PRIV_QUEUE(c_p, 0);
    if (sigp && ERTS_SIG_IS_RECV_MARKER(sigp))
        sigpp = erts_msgq_pass_recv_markers(c_p, sigpp);
    c_p->sig_qs.save = sigpp;
    ERTS_HDBG_CHECK_SIGNAL_PRIV_QUEUE(c_p, 0);
}
ERTS_GLB_INLINE void
erts_msgq_set_save_end(Process *c_p)
{
    ASSERT(!(c_p->sig_qs.flags & FS_HANDLING_SIGS));
    erts_proc_sig_queue_lock(c_p);
    erts_proc_sig_fetch(c_p);
    erts_proc_unlock(c_p, ERTS_PROC_LOCK_MSGQ);
    ASSERT(ERTS_MQ_GET_SAVE_INFO(c_p) == FS_SET_SAVE_INFO_FIRST);
    if (!c_p->sig_qs.cont) {
        c_p->sig_qs.save = c_p->sig_qs.last;
        c_p->sig_qs.flags &= ~FS_PRIO_MQ_SAVE;
    }
    else {
	erts_msgq_recv_marker_create_insert_set_save(c_p, NIL);
    }
    ERTS_MQ_SET_SAVE_INFO(c_p, FS_SET_SAVE_INFO_LAST);
}
#undef ERTS_PROC_SIG_RECV_MARK_CLEAR_OLD_MARK__
ERTS_GLB_INLINE erts_aint32_t
erts_proc_sig_check_wait_dirty_handle_signals(Process *c_p,
                                              erts_aint32_t state_in)
{
    erts_aint32_t state = state_in;
    ASSERT(!!erts_get_scheduler_data());
    ERTS_LC_ASSERT(ERTS_PROC_LOCK_MAIN == erts_proc_lc_my_proc_locks(c_p));
    if (c_p->sig_qs.flags & FS_HANDLING_SIGS) {
        erts_proc_sig_do_wait_dirty_handle_signals__(c_p);
        state = erts_atomic32_read_mb(&c_p->state);
    }
    ERTS_LC_ASSERT(ERTS_PROC_LOCK_MAIN == erts_proc_lc_my_proc_locks(c_p));
    ASSERT(!(c_p->sig_qs.flags & FS_HANDLING_SIGS));
    return state;
}
#endif
#endif