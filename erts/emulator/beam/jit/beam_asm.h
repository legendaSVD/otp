#if defined(BEAMASM) && !defined(__BEAM_ASM_H__)
#    define __BEAM_ASM_H__
#    include "sys.h"
#    include "bif.h"
#    include "erl_fun.h"
#    include "erl_process.h"
#    include "beam_bp.h"
#    include "beam_code.h"
#    include "beam_file.h"
#    include "beam_common.h"
#    if defined(__APPLE__)
#        include <libkern/OSCacheControl.h>
#    endif
#    ifdef HAVE_LINUX_PERF_SUPPORT
enum beamasm_perf_flags {
    BEAMASM_PERF_DUMP = (1 << 0),
    BEAMASM_PERF_MAP = (1 << 1),
    BEAMASM_PERF_FP = (1 << 2),
    BEAMASM_PERF_ENABLED =
            BEAMASM_PERF_DUMP | BEAMASM_PERF_MAP | BEAMASM_PERF_FP,
    BEAMASM_PERF_DISABLED = 0,
};
extern enum beamasm_perf_flags erts_jit_perf_support;
extern char etrs_jit_perf_directory[MAXPATHLEN];
#    endif
extern int erts_jit_single_map;
void beamasm_init(void);
void *beamasm_new_assembler(Eterm mod,
                            int num_labels,
                            int num_functions,
                            BeamFile *beam);
void beamasm_codegen(void *ba,
                     const void **executable_region,
                     void **writable_region,
                     const BeamCodeHeader *in_hdr,
                     const BeamCodeHeader **out_exec_hdr,
                     BeamCodeHeader **out_rw_hdr);
void *beamasm_register_metadata(void *ba, const BeamCodeHeader *header);
void beamasm_unregister_metadata(void *handle);
void beamasm_purge_module(const void *executable_region,
                          void *writable_region,
                          size_t size);
void beamasm_delete_assembler(void *ba);
int beamasm_emit(void *ba, unsigned specific_op, BeamOp *op);
void beamasm_emit_coverage(void *instance,
                           void *coverage,
                           Uint index,
                           Uint size);
ErtsCodePtr beamasm_get_code(void *ba, int label);
ErtsCodePtr beamasm_get_lambda(void *ba, int index);
const byte *beamasm_get_rodata(void *ba, char *label);
void beamasm_embed_rodata(void *ba,
                          const char *labelName,
                          const char *buff,
                          size_t size);
void beamasm_embed_bss(void *ba, char *labelName, size_t size);
unsigned int beamasm_patch_catches(void *ba, char *rw_base);
void beamasm_patch_import(void *ba,
                          char *rw_base,
                          int index,
                          const Export *import);
void beamasm_patch_literal(void *ba, char *rw_base, int index, Eterm lit);
void beamasm_patch_lambda(void *ba,
                          char *rw_base,
                          int index,
                          const ErlFunEntry *fe);
void beamasm_patch_strings(void *ba, char *rw_base, const byte *strtab);
void beamasm_emit_call_nif(const ErtsCodeInfo *info,
                           void *normal_fptr,
                           void *lib,
                           void *dirty_fptr,
                           char *buff,
                           unsigned buff_len);
Uint beamasm_get_header(void *ba, const BeamCodeHeader **);
const ErtsCodeInfo *beamasm_get_on_load(void *ba);
char *beamasm_get_base(void *instance);
size_t beamasm_get_offset(void *ba);
enum erts_is_line_breakpoint beamasm_is_line_breakpoint_trampoline(
        ErtsCodePtr addr);
void beamasm_unseal_module(const void *executable_region,
                           void *writable_region,
                           size_t size);
void beamasm_seal_module(const void *executable_region,
                         void *writable_region,
                         size_t size);
void beamasm_flush_icache(const void *address, size_t size);
#    if defined(__aarch64__)
#        define BEAM_ASM_FUNC_PROLOGUE_SIZE 12
#    else
#        define BEAM_ASM_FUNC_PROLOGUE_SIZE 8
#    endif
#    if defined(__aarch64__)
#        define BEAM_ASM_NFUNC_SIZE (BEAM_ASM_FUNC_PROLOGUE_SIZE + 4)
#    else
#        define BEAM_ASM_NFUNC_SIZE (BEAM_ASM_FUNC_PROLOGUE_SIZE + 8)
#    endif
enum erts_asm_bp_flag {
    ERTS_ASM_BP_FLAG_NONE = 0,
    ERTS_ASM_BP_FLAG_CALL_NIF_EARLY = 1 << 0,
    ERTS_ASM_BP_FLAG_BP = 1 << 1,
    ERTS_ASM_BP_FLAG_BP_NIF_CALL_NIF_EARLY =
            ERTS_ASM_BP_FLAG_CALL_NIF_EARLY | ERTS_ASM_BP_FLAG_BP
};
static inline enum erts_asm_bp_flag erts_asm_bp_get_flags(
        const ErtsCodeInfo *ci_exec) {
    return (enum erts_asm_bp_flag)ci_exec->u.metadata.breakpoint_flag;
}
static inline void erts_asm_bp_enable(ErtsCodePtr rw_p) {
#    if defined(__aarch64__)
    Uint32 volatile *rw_code = (Uint32 *)rw_p;
    ASSERT(rw_code[0] == 0x14000002);
    rw_code[0] = 0x14000001;
#    else
    byte volatile *rw_code = (byte *)rw_p;
    ASSERT(rw_code[0] == 0xEB && rw_code[1] == 0x06 && rw_code[2] == 0x90 &&
           rw_code[3] == 0xE8);
    rw_code[1] = 0x01;
#    endif
}
static inline void erts_asm_bp_disable(ErtsCodePtr rw_p) {
#    if defined(__aarch64__)
    Uint32 volatile *rw_code = (Uint32 *)rw_p;
    ASSERT(rw_code[0] == 0x14000001);
    rw_code[0] = 0x14000002;
#    else
    byte volatile *rw_code = (byte *)rw_p;
    ASSERT(rw_code[0] == 0xEB && rw_code[1] == 0x01 && rw_code[2] == 0x90 &&
           rw_code[3] == 0xE8);
    rw_code[1] = 0x06;
#    endif
}
static inline void erts_asm_bp_set_flag(ErtsCodeInfo *ci_rw,
                                        const ErtsCodeInfo *ci_exec,
                                        enum erts_asm_bp_flag flag) {
    ASSERT(flag != ERTS_ASM_BP_FLAG_NONE);
    (void)ci_exec;
    if (ci_rw->u.metadata.breakpoint_flag == ERTS_ASM_BP_FLAG_NONE) {
        ErtsCodePtr rw_p = erts_codeinfo_to_code(ci_rw);
#    if defined(__aarch64__)
        rw_p = (ErtsCodePtr)((Uint32 *)rw_p + 1);
#    endif
        erts_asm_bp_enable(rw_p);
    }
    ci_rw->u.metadata.breakpoint_flag |= flag;
}
static inline void erts_asm_bp_unset_flag(ErtsCodeInfo *ci_rw,
                                          const ErtsCodeInfo *ci_exec,
                                          enum erts_asm_bp_flag flag) {
    ASSERT(flag != ERTS_ASM_BP_FLAG_NONE);
    (void)ci_exec;
    ci_rw->u.metadata.breakpoint_flag &= ~flag;
    if (ci_rw->u.metadata.breakpoint_flag == ERTS_ASM_BP_FLAG_NONE) {
        ErtsCodePtr rw_p = erts_codeinfo_to_code(ci_rw);
#    if defined(__aarch64__)
        rw_p = (ErtsCodePtr)((Uint32 *)rw_p + 1);
#    endif
        erts_asm_bp_disable(rw_p);
    }
}
#endif