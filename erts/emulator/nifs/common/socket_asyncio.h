#ifndef SOCKET_ASYNCIO_H__
#define SOCKET_ASYNCIO_H__
#include "socket_io.h"
extern int  esaio_init(unsigned int     numThreads,
                       const ESockData* dataP);
extern void esaio_finish(void);
extern ERL_NIF_TERM esaio_info(ErlNifEnv* env);
extern ERL_NIF_TERM esaio_command(ErlNifEnv*   env,
                                  ERL_NIF_TERM command,
                                  ERL_NIF_TERM cdata);
extern ERL_NIF_TERM esaio_open_plain(ErlNifEnv*       env,
                                     int              domain,
                                     int              type,
                                     int              protocol,
                                     ERL_NIF_TERM     eopts,
                                     const ESockData* dataP);
extern ERL_NIF_TERM esaio_bind(ErlNifEnv*       env,
                               ESockDescriptor* descP,
                               ESockAddress*    sockAddrP,
                               SOCKLEN_T        addrLen);
extern ERL_NIF_TERM esaio_connect(ErlNifEnv*       env,
                                  ESockDescriptor* descP,
                                  ERL_NIF_TERM     sockRef,
                                  ERL_NIF_TERM     connRef,
                                  ESockAddress*    addrP,
                                  SOCKLEN_T        addrLen);
extern ERL_NIF_TERM esaio_accept(ErlNifEnv*       env,
                                 ESockDescriptor* descP,
                                 ERL_NIF_TERM     sockRef,
                                 ERL_NIF_TERM     accRef);
extern ERL_NIF_TERM esaio_send(ErlNifEnv*       env,
                               ESockDescriptor* descP,
                               ERL_NIF_TERM     sockRef,
                               ERL_NIF_TERM     sendRef,
                               ErlNifBinary*    sndDataP,
                               int              flags);
extern ERL_NIF_TERM esaio_sendto(ErlNifEnv*       env,
                                 ESockDescriptor* descP,
                                 ERL_NIF_TERM     sockRef,
                                 ERL_NIF_TERM     sendRef,
                                 ErlNifBinary*    dataP,
                                 int              flags,
                                 ESockAddress*    toAddrP,
                                 SOCKLEN_T        toAddrLen);
extern ERL_NIF_TERM esaio_sendmsg(ErlNifEnv*       env,
                                  ESockDescriptor* descP,
                                  ERL_NIF_TERM     sockRef,
                                  ERL_NIF_TERM     sendRef,
                                  ERL_NIF_TERM     eMsg,
                                  int              flags,
                                  ERL_NIF_TERM     eIOV,
                                  const ESockData* dataP);
extern ERL_NIF_TERM esaio_sendv(ErlNifEnv*       env,
                                ESockDescriptor* descP,
                                ERL_NIF_TERM     sockRef,
                                ERL_NIF_TERM     sendRef,
                                ERL_NIF_TERM     eIOV,
                                const ESockData* dataP);
extern ERL_NIF_TERM esaio_recv(ErlNifEnv*       env,
                               ESockDescriptor* descP,
                               ERL_NIF_TERM     sockRef,
                               ERL_NIF_TERM     recvRef,
                               ssize_t          len,
                               int              flags);
extern ERL_NIF_TERM esaio_recvfrom(ErlNifEnv*       env,
                                   ESockDescriptor* descP,
                                   ERL_NIF_TERM     sockRef,
                                   ERL_NIF_TERM     recvRef,
                                   ssize_t          len,
                                   int              flags);
extern ERL_NIF_TERM esaio_recvmsg(ErlNifEnv*       env,
                                  ESockDescriptor* descP,
                                  ERL_NIF_TERM     sockRef,
                                  ERL_NIF_TERM     recvRef,
                                  ssize_t          bufLen,
                                  ssize_t          ctrlLen,
                                  int              flags);
extern ERL_NIF_TERM esaio_close(ErlNifEnv*       env,
                                ESockDescriptor* descP);
extern ERL_NIF_TERM esaio_fin_close(ErlNifEnv*       env,
                                    ESockDescriptor* descP);
extern ERL_NIF_TERM esaio_shutdown(ErlNifEnv*       env,
                                   ESockDescriptor* descP,
                                   int              how);
extern ERL_NIF_TERM esaio_sockname(ErlNifEnv*       env,
                                   ESockDescriptor* descP);
extern ERL_NIF_TERM esaio_peername(ErlNifEnv*       env,
                                   ESockDescriptor* descP);
extern ERL_NIF_TERM esaio_cancel_connect(ErlNifEnv*       env,
                                         ESockDescriptor* descP,
                                         ERL_NIF_TERM     opRef);
extern ERL_NIF_TERM esaio_cancel_accept(ErlNifEnv*       env,
                                        ESockDescriptor* descP,
                                        ERL_NIF_TERM     sockRef,
                                        ERL_NIF_TERM     opRef);
extern ERL_NIF_TERM esaio_cancel_send(ErlNifEnv*       env,
                                      ESockDescriptor* descP,
                                      ERL_NIF_TERM     sockRef,
                                      ERL_NIF_TERM     opRef);
extern ERL_NIF_TERM esaio_cancel_recv(ErlNifEnv*       env,
                                      ESockDescriptor* descP,
                                      ERL_NIF_TERM     sockRef,
                                      ERL_NIF_TERM     opRef);
extern ERL_NIF_TERM esaio_ioctl3(ErlNifEnv*       env,
                                 ESockDescriptor* descP,
                                 unsigned long    req,
                                 ERL_NIF_TERM     arg);
extern ERL_NIF_TERM esaio_ioctl2(ErlNifEnv*       env,
                                 ESockDescriptor* descP,
                                 unsigned long    req);
extern void esaio_dtor(ErlNifEnv*       env,
                       ESockDescriptor* descP);
extern void esaio_stop(ErlNifEnv*       env,
                       ESockDescriptor* descP);
extern void esaio_down(ErlNifEnv*           env,
                       ESockDescriptor*     descP,
                       const ErlNifPid*     pidP,
                       const ErlNifMonitor* monP);
extern void esaio_down_ctrl(ErlNifEnv*       env,
                            ESockDescriptor* descP,
                            const ErlNifPid* pidP);
#endif