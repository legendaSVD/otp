#ifndef _ERL_DEBUGGER_H_
#define _ERL_DEBUGGER_H_
#define ERTS_DEBUGGER_ENABLED          ((Uint)1 << 0)
#define ERTS_DEBUGGER_LINE_BREAKPOINTS ((Uint)1 << 1)
#define ERTS_DEBUGGER_IS_ENABLED_IN(Var, Flgs) \
    ((Var & (Flgs | ERTS_DEBUGGER_ENABLED)) == (Flgs | ERTS_DEBUGGER_ENABLED))
#define ERTS_DEBUGGER_IS_ENABLED(Flgs) \
    ERTS_DEBUGGER_IS_ENABLED_IN(erts_debugger_flags, Flgs)
extern Uint erts_debugger_flags;
void erts_init_debugger(void);
int erts_send_debugger_event(Process *c_p, Eterm event);
#endif