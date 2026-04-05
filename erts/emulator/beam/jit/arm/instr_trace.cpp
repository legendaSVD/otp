#include "beam_asm.hpp"
extern "C"
{
#include "beam_common.h"
#include "erl_bif_table.h"
#include "beam_bp.h"
};
void BeamGlobalAssembler::emit_generic_bp_global() {
    emit_enter_erlang_frame();
    lea(ARG2, a64::Mem(ARG1, offsetof(Export, info)));
    emit_enter_runtime<Update::eHeapAlloc | Update::eXRegs |
                       Update::eReductions>();
    a.mov(ARG1, c_p);
    load_x_reg_array(ARG3);
    runtime_call<BeamInstr (*)(Process *, ErtsCodeInfo *, Eterm *),
                 erts_generic_breakpoint>();
    emit_leave_runtime<Update::eHeapAlloc | Update::eXRegs |
                       Update::eReductions>();
    emit_leave_erlang_frame();
    a.br(ARG1);
}
void BeamGlobalAssembler::emit_generic_bp_local() {
    a.ldr(ARG2, a64::Mem(a64::sp, 8));
    a.str(ARG2, TMP_MEM1q);
    a.sub(ARG2, ARG2, imm(BEAM_ASM_FUNC_PROLOGUE_SIZE + sizeof(ErtsCodeInfo)));
    emit_enter_runtime_frame();
    emit_enter_runtime<Update::eHeapAlloc | Update::eXRegs |
                       Update::eReductions>();
    a.mov(ARG1, c_p);
    load_x_reg_array(ARG3);
    runtime_call<BeamInstr (*)(Process *, ErtsCodeInfo *, Eterm *),
                 erts_generic_breakpoint>();
    emit_leave_runtime<Update::eHeapAlloc | Update::eXRegs |
                       Update::eReductions>();
    a.cmp(ARG1, imm(BeamOpCodeAddr(op_i_debug_breakpoint)));
    a.b_eq(labels[debug_bp]);
    emit_leave_runtime_frame();
    a.ret(a64::x30);
}
void BeamGlobalAssembler::emit_debug_bp() {
    Label error = a.new_label();
    a.ldr(ARG2, TMP_MEM1q);
    a.sub(ARG2, ARG2, imm(BEAM_ASM_FUNC_PROLOGUE_SIZE + sizeof(ErtsCodeMFA)));
    emit_enter_runtime<Update::eHeapAlloc | Update::eXRegs |
                       Update::eReductions>();
    a.mov(ARG1, c_p);
    load_x_reg_array(ARG3);
    a.mov(ARG4, imm(am_breakpoint));
    runtime_call<
            const Export *(*)(Process *, const ErtsCodeMFA *, Eterm *, Eterm),
            call_error_handler>();
    emit_leave_runtime<Update::eHeapAlloc | Update::eXRegs |
                       Update::eReductions>();
    emit_leave_runtime_frame();
    emit_leave_runtime_frame();
    a.cbz(ARG1, error);
    emit_leave_erlang_frame();
    branch(emit_setup_dispatchable_call(ARG1));
    a.bind(error);
    {
        a.ldr(ARG2, TMP_MEM1q);
        mov_imm(ARG4, 0);
        a.b(labels[raise_exception_shared]);
    }
}
static void return_trace(Process *c_p,
                         ErtsCodeMFA *mfa,
                         Eterm val,
                         ErtsTracer tracer,
                         Eterm session_id) {
    ERTS_UNREQ_PROC_MAIN_LOCK(c_p);
    erts_trace_return(c_p, mfa, val, tracer, session_id);
    ERTS_REQ_PROC_MAIN_LOCK(c_p);
}
void BeamModuleAssembler::emit_return_trace() {
    a.ldr(ARG2, getYRef(0));
    a.mov(ARG3, XREG0);
    a.ldr(ARG4, getYRef(1));
    a.ldr(ARG5, getYRef(2));
    ERTS_CT_ASSERT(ERTS_HIGHEST_CALLEE_SAVE_XREG >= 1);
    emit_enter_runtime<Update::eHeapAlloc>(1);
    a.mov(ARG1, c_p);
    runtime_call<void (*)(Process *, ErtsCodeMFA *, Eterm, ErtsTracer, Eterm),
                 return_trace>();
    emit_leave_runtime<Update::eHeapAlloc>(1);
    emit_deallocate(ArgVal(ArgVal::Type::Word, BEAM_RETURN_TRACE_FRAME_SZ));
    emit_return_do(true);
}
void BeamModuleAssembler::emit_i_call_trace_return() {
    a.ldr(ARG2, getYRef(0));
    mov_imm(ARG4, 0);
    a.tst(ARG2, imm(_CPMASK));
    a.sub(ARG2, ARG2, imm(sizeof(ErtsCodeInfo)));
    a.csel(ARG2, ARG2, ARG4, arm::CondCode::kEQ);
    a.ldr(ARG3, getYRef(1));
    a.ldr(ARG4, getYRef(2));
    ERTS_CT_ASSERT(ERTS_HIGHEST_CALLEE_SAVE_XREG >= 1);
    emit_enter_runtime<Update::eHeapAlloc>(1);
    a.mov(ARG1, c_p);
    runtime_call<void (*)(Process *, const ErtsCodeInfo *, Eterm, Eterm),
                 erts_call_trace_return>();
    emit_leave_runtime<Update::eHeapAlloc>(1);
    emit_deallocate(
            ArgVal(ArgVal::Type::Word, BEAM_RETURN_CALL_ACC_TRACE_FRAME_SZ));
    emit_return_do(true);
}
void BeamModuleAssembler::emit_i_return_to_trace() {
    a.ldr(ARG2, getYRef(0));
    a.add(ARG3, E, imm(BEAM_RETURN_TO_TRACE_FRAME_SZ * sizeof(Eterm)));
    ERTS_CT_ASSERT(ERTS_HIGHEST_CALLEE_SAVE_XREG >= 1);
    emit_enter_runtime<Update::eHeapAlloc>(1);
    a.mov(ARG1, c_p);
    runtime_call<void (*)(Process *, Eterm, Eterm *),
                 beam_jit_return_to_trace>();
    emit_leave_runtime<Update::eHeapAlloc>(1);
    emit_deallocate(ArgVal(ArgVal::Type::Word, BEAM_RETURN_TO_TRACE_FRAME_SZ));
    emit_return_do(true);
}
void BeamModuleAssembler::emit_i_hibernate() {
    emit_enter_runtime<Update::eReductions | Update::eHeap | Update::eStack>(0);
    a.mov(ARG1, c_p);
    load_x_reg_array(ARG2);
    mov_imm(ARG3, 0);
    runtime_call<void (*)(Process *, Eterm *, int), erts_hibernate>();
    emit_leave_runtime<Update::eReductions | Update::eHeap | Update::eStack>(0);
    a.ldr(TMP1.w(), a64::Mem(c_p, offsetof(Process, flags)));
    a.and_(TMP1, TMP1, imm(~F_HIBERNATE_SCHED));
    a.str(TMP1.w(), a64::Mem(c_p, offsetof(Process, flags)));
    mov_imm(XREG0, am_ok);
    fragment_call(ga->get_dispatch_return());
}