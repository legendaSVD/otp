#ifndef SOCKET_TARRAY_H__
#define SOCKET_TARRAY_H__
typedef void* SocketTArray;
extern SocketTArray esock_tarray_create(Uint32 sz);
extern void         esock_tarray_delete(SocketTArray ta);
extern Uint32       esock_tarray_sz(SocketTArray ta);
extern void         esock_tarray_add(SocketTArray ta, ERL_NIF_TERM t);
extern void         esock_tarray_tolist(SocketTArray  ta,
                                        ErlNifEnv*    env,
                                        ERL_NIF_TERM* list);
#define TARRAY_CREATE(SZ)        esock_tarray_create((SZ))
#define TARRAY_DELETE(TA)        esock_tarray_delete((TA))
#define TARRAY_SZ(TA)            esock_tarray_sz((TA))
#define TARRAY_ADD(TA, T)        esock_tarray_add((TA), (T))
#define TARRAY_TOLIST(TA, E, L)  esock_tarray_tolist((TA), (E), (L))
#endif