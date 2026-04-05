#ifndef __EXPORT_H__
#define __EXPORT_H__
#include "sys.h"
#include "index.h"
#include "code_ix.h"
#ifdef BEAMASM
#define OP_PAD BeamInstr __pad[1];
#else
#define OP_PAD
#endif
typedef struct export_
{
    ErtsDispatchable dispatch;
    int bif_number;
    int is_bif_traced;
    Eterm lambda;
    ErtsCodeInfo info;
    union {
        struct {
            OP_PAD
            BeamInstr op;
        } common;
        struct {
            OP_PAD
            BeamInstr op;
            BeamInstr address;
        } bif;
        struct {
            OP_PAD
            BeamInstr op;
            BeamInstr address;
        } breakpoint;
        struct {
            OP_PAD
            BeamInstr op;
            BeamInstr deferred;
        } not_loaded;
    } trampoline;
} Export;
#if defined(DEBUG)
#define DBG_CHECK_EXPORT(EP, CX) \
    do { \
        if(erts_is_export_trampoline_active((EP), (CX))) { \
 \
            ASSERT(((BeamIsOpCode((EP)->trampoline.common.op, op_i_generic_breakpoint)) && \
                    (EP)->trampoline.breakpoint.address != 0) || \
                    \
                   (BeamIsOpCode((EP)->trampoline.common.op, op_call_error_handler))); \
        } \
    } while(0)
#else
#define DBG_CHECK_EXPORT(EP, CX)
#endif
void init_export_table(void);
void export_info(fmtfn_t, void *);
ERTS_GLB_INLINE void erts_activate_export_trampoline(Export *ep, int code_ix);
ERTS_GLB_INLINE int erts_is_export_trampoline_active(const Export * const ep, int code_ix);
ERTS_GLB_INLINE const Export *erts_active_export_entry(Eterm m,
                                                       Eterm f,
                                                       unsigned a);
Export* erts_export_put(Eterm mod, Eterm func, unsigned int arity);
Export* erts_export_get_or_make_stub(Eterm, Eterm, unsigned);
Export *export_list(int,ErtsCodeIndex);
int export_list_size(ErtsCodeIndex);
int export_table_sz(void);
int export_entries_sz(void);
const Export *export_get(const Export*);
void export_start_staging(void);
void export_end_staging(int commit);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE void erts_activate_export_trampoline(Export *ep, int code_ix) {
    ErtsCodePtr trampoline_address;
#ifdef BEAMASM
    extern ErtsCodePtr beam_export_trampoline;
    trampoline_address = beam_export_trampoline;
#else
    trampoline_address = (ErtsCodePtr)&ep->trampoline;
#endif
    ep->dispatch.addresses[code_ix] = trampoline_address;
}
ERTS_GLB_INLINE int erts_is_export_trampoline_active(const Export * const ep, int code_ix) {
    ErtsCodePtr trampoline_address;
#ifdef BEAMASM
    extern ErtsCodePtr beam_export_trampoline;
    trampoline_address = beam_export_trampoline;
#else
    trampoline_address = (ErtsCodePtr)&ep->trampoline;
#endif
    return ep->dispatch.addresses[code_ix] == trampoline_address;
}
ERTS_GLB_INLINE const Export *
erts_active_export_entry(Eterm m, Eterm f, unsigned int a)
{
    extern const Export *erts_find_export_entry(Eterm m,
                                                Eterm f,
                                                unsigned a,
                                                ErtsCodeIndex);
    return erts_find_export_entry(m, f, a, erts_active_code_ix());
}
#endif
#endif