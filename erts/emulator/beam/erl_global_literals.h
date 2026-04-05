#ifndef __ERL_GLOBAL_LITERALS_H__
#define __ERL_GLOBAL_LITERALS_H__
extern Eterm ERTS_GLOBAL_LIT_OS_TYPE;
extern Eterm ERTS_GLOBAL_LIT_OS_VERSION;
extern Eterm ERTS_GLOBAL_LIT_DFLAGS_RECORD;
extern Eterm ERTS_GLOBAL_LIT_ERL_FILE_SUFFIX;
extern Eterm ERTS_GLOBAL_LIT_EMPTY_TUPLE;
void init_global_literals(void);
Eterm *erts_global_literal_allocate(Uint sz, struct erl_off_heap_header ***ohp);
void erts_global_literal_register(Eterm *variable);
ErtsLiteralArea *erts_global_literal_iterate_area(ErtsLiteralArea *prev);
#endif