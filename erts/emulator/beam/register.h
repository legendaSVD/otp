#ifndef __REGPROC_H__
#define __REGPROC_H__
#include "sys.h"
#include "hash.h"
#include "erl_process.h"
#define ERL_PORT_GET_PORT_TYPE_ONLY__
#include "erl_port.h"
#undef ERL_PORT_GET_PORT_TYPE_ONLY__
typedef struct reg_proc
{
    HashBucket bucket;
    Process *p;
    Port *pt;
    Eterm name;
} RegProc;
int process_reg_sz(void);
void init_register_table(void);
void register_info(fmtfn_t, void *);
int erts_register_name(Process *, Eterm, Eterm);
Eterm erts_whereis_name_to_id(Process *, Eterm);
void erts_whereis_name(Process *, ErtsProcLocks,
		       Eterm, Process**, ErtsProcLocks, int,
		       Port**, int);
Process *erts_whereis_process(Process *,
			      ErtsProcLocks,
			      Eterm,
			      ErtsProcLocks,
			      int);
int erts_unregister_name(Process *, ErtsProcLocks, Port *, Eterm);
#endif