#ifndef _BEAM_CODE_H
#define _BEAM_CODE_H
#include "sys.h"
#include "erl_process.h"
#include "erl_md5.h"
#define LINE_INVALID_LOCATION (0)
#define IS_VALID_LOCATION(File, Line) \
    ((unsigned) (File) < 255 && (unsigned) (Line) < ((1 << 24) - 1))
#define MAKE_LOCATION(File, Line)        \
    (IS_VALID_LOCATION((File), (Line)) ? \
        (((File) << 24) | (Line)) :      \
        LINE_INVALID_LOCATION)
#define LOC_FILE(Loc) ((Loc) >> 24)
#define LOC_LINE(Loc) ((Loc) & ((1 << 24)-1))
#ifdef BEAMASM
#  define BEAM_NATIVE_MIN_FUNC_SZ 30
#else
#  define BEAM_NATIVE_MIN_FUNC_SZ 4
#endif
#define MD5_SIZE MD5_DIGEST_LENGTH
typedef struct BeamCodeLineTab_ BeamCodeLineTab;
typedef struct BeamDebugTab_ BeamDebugTab;
typedef struct beam_code_header {
    UWord num_functions;
    const byte *attr_ptr;
    UWord attr_size;
    UWord attr_size_on_heap;
    const byte *compile_ptr;
    UWord compile_size;
    UWord compile_size_on_heap;
    struct ErtsLiteralArea_ *literal_area;
    const ErtsCodeInfo *on_load;
    const BeamCodeLineTab *line_table;
#ifdef BEAMASM
    Uint coverage_mode;
    void *coverage;
    byte *line_coverage_valid;
    Uint32 *loc_index_to_cover_id;
    Uint line_coverage_len;
    const BeamDebugTab *debug;
#endif
    Uint debugger_flags;
    const byte *md5_ptr;
    byte* are_nifs;
    const ErtsCodeInfo *functions[1];
} BeamCodeHeader;
struct BeamCodeLineTab_ {
    const Eterm* fname_ptr;
    int loc_size;
    union {
        Uint16* p2;
        Uint32* p4;
    } loc_tab;
    const void** func_tab[];
};
typedef struct {
    Uint32 location_index;
    Sint16 frame_size;
    Uint16 num_vars;
    Uint32 num_calls_terms;
    Eterm *first;
} BeamDebugItem;
struct BeamDebugTab_ {
    Uint32 item_count;
    BeamDebugItem *items;
};
extern Uint erts_total_code_size;
struct ErtsLiteralArea_;
void erts_release_literal_area(struct ErtsLiteralArea_* literal_area);
struct erl_fun_entry;
void erts_purge_state_add_fun(struct erl_fun_entry *fe);
const Export *
erts_suspend_process_on_pending_purge_lambda(Process *c_p,
                                             const struct erl_fun_entry*);
#ifdef  ENABLE_DBG_TRACE_MFA
void dbg_set_traced_mfa(const char* m, const char* f, Uint a);
int dbg_is_traced_mfa(Eterm m, Eterm f, Uint a);
void dbg_vtrace_mfa(unsigned ix, const char* format, ...);
#define DBG_TRACE_MFA(M,F,A,FMT, ...) do {\
    unsigned ix;\
    if ((ix=dbg_is_traced_mfa(M,F,A))) \
        dbg_vtrace_mfa(ix, FMT"\n", ##__VA_ARGS__);\
  }while(0)
#define DBG_TRACE_MFA_P(MFA, FMT, ...) \
        DBG_TRACE_MFA((MFA)->module, (MFA)->function, (MFA)->arity, FMT, ##__VA_ARGS__)
#else
#  define dbg_set_traced_mfa(M,F,A)
#  define DBG_TRACE_MFA(M,F,A,FMT, ...)
#  define DBG_TRACE_MFA_P(MFA,FMT, ...)
#endif
#endif