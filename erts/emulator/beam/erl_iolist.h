#if !defined(ERL_IOLIST_H)
#    define ERL_IOLIST_H
#include "sys.h"
#include "global.h"
#define ERTS_IOLIST_TO_BUF_OVERFLOW	(~((ErlDrvSizeT) 0))
#define ERTS_IOLIST_TO_BUF_TYPE_ERROR	(~((ErlDrvSizeT) 1))
#define ERTS_IOLIST_TO_BUF_YIELD	(~((ErlDrvSizeT) 2))
#define ERTS_IOLIST_TO_BUF_FAILED(R) \
    (((R) & (~((ErlDrvSizeT) 3))) == (~((ErlDrvSizeT) 3)))
#define ERTS_IOLIST_TO_BUF_SUCCEEDED(R) \
    (!ERTS_IOLIST_TO_BUF_FAILED((R)))
void erts_init_iolist(void);
ErlDrvSizeT erts_iolist_to_buf(Eterm obj, char *data, ErlDrvSizeT size);
int erts_iolist_size(Eterm, ErlDrvSizeT *);
#endif