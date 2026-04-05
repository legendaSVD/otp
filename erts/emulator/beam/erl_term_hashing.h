#ifndef ERL_TERM_HASHING_H__
#define ERL_TERM_HASHING_H__
#include "sys.h"
#include "erl_drv_nif.h"
typedef UWord erts_ihash_t;
erts_ihash_t erts_internal_salted_hash(Eterm term, erts_ihash_t salt);
erts_ihash_t erts_internal_hash(Eterm term);
erts_ihash_t erts_map_hash(Eterm term);
#ifdef DEBUG
#  define DBG_HASHMAP_COLLISION_BONANZA
#endif
#ifdef DBG_HASHMAP_COLLISION_BONANZA
erts_ihash_t erts_dbg_hashmap_collision_bonanza(erts_ihash_t hash, Eterm key);
#endif
typedef struct {
    Uint32 a,b,c;
} ErtsBlockHashHelperCtx;
typedef struct {
    ErtsBlockHashHelperCtx hctx;
    const byte *ptr;
    Uint len;
    Uint tot_len;
} ErtsBlockHashState;
typedef struct {
    ErtsBlockHashHelperCtx hctx;
    SysIOVec* iov;
    Uint vlen;
    Uint tot_len;
    int vix;
    int ix;
} ErtsIovBlockHashState;
Uint32 make_hash2(Eterm);
Uint32 trapping_make_hash2(Eterm, Eterm*, struct process*);
Uint32 make_hash(Eterm);
void erts_block_hash_init(ErtsBlockHashState *state,
                          const byte *ptr,
                          Uint len,
                          Uint32 initval);
int erts_block_hash(Uint32 *hashp,
                    Uint *sizep,
                    ErtsBlockHashState *state);
void erts_iov_block_hash_init(ErtsIovBlockHashState *state,
                              SysIOVec *iov,
                              Uint vlen,
                              Uint32 initval);
int erts_iov_block_hash(Uint32 *hashp,
                        Uint *sizep,
                        ErtsIovBlockHashState *state);
#endif