#ifndef ERL_DYN_LOCK_CHECK_H__
#define ERL_DYN_LOCK_CHECK_H__
#ifdef ERTS_ENABLE_LOCK_CHECK
# define ERTS_DYN_LOCK_CHECK
#endif
#ifdef ERTS_DYN_LOCK_CHECK
typedef struct {
    unsigned ix;
} erts_dlc_t;
void erts_dlc_create_lock(erts_dlc_t* dlc, const char* name);
int erts_dlc_lock(erts_dlc_t* dlc);
void erts_dlc_trylock(erts_dlc_t* dlc, int locked);
void erts_dlc_unlock(erts_dlc_t* dlc);
void erts_dlc_init(void);
#endif
#endif