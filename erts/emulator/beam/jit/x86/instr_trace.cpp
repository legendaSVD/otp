#include "beam_asm.hpp"
extern "C"
{
#include "beam_common.h"
#include "erl_bif_table.h"
#include "beam_bp.h"
};
void BeamGlobalAssembler::emit_generic_bp_global() {
    emit_enter_frame();
    emit_enter_runtime<Update::eReductions | Update::eHeapAlloc>();
    a.mov(ARG1, c_p);
    a.lea(ARG2, x86::qword_ptr(RET, offsetof(Export, info)));
    load_x_reg_array(ARG3);
    runtime_call<BeamInstr (*)(Process *, ErtsCodeInfo *, Eterm *),
                 erts_generic_breakpoint>();
    emit_leave_runtime<Update::eReductions | Update::eHeapAlloc>();
    emit_leave_frame();
    a.jmp(RET);
}
void BeamGlobalAssembler::emit_generic_bp_local() {
    emit_assert_erlang_stack();
#ifdef NATIVE_ERLANG_STACK
    a.pop(TMP_MEM2q);
    a.pop(ARG2);
#else
    a.mov(ARG2, x86::qword_ptr(x86::rsp, 8));
#endif
    a.mov(TMP_MEM1q, ARG2);
    a.sub(ARG2, imm(sizeof(UWord) + sizeof(ErtsCodeInfo)));
#ifdef DEBUG
    {
        Label next = a.new_label();
        a.test(ARG2, imm(sizeof(UWord) - 1));
        a.je(next);
        a.hlt();
        a.bind(next);
    }
#endif
    emit_enter_frame();
    emit_enter_runtime<Update::eReductions | Update::eHeapAlloc>();
    a.mov(ARG1, c_p);
    load_x_reg_array(ARG3);
    runtime_call<BeamInstr (*)(Process *, ErtsCodeInfo *, Eterm *),
                 erts_generic_breakpoint>();
    emit_leave_runtime<Update::eReductions | Update::eHeapAlloc>();
    emit_leave_frame();
    a.cmp(RET, imm(BeamOpCodeAddr(op_i_debug_breakpoint)));
    a.je(labels[debug_bp]);
#ifdef NATIVE_ERLANG_STACK
    a.push(TMP_MEM1q);
    a.push(TMP_MEM2q);
#endif
    a.ret();
}
void BeamGlobalAssembler::emit_debug_bp() {
    Label error = a.new_label();
#ifndef NATIVE_ERLANG_STACK
    a.add(x86::rsp, imm(sizeof(ErtsCodePtr[2])));
#endif
    emit_assert_erlang_stack();
    emit_enter_frame();
    emit_enter_runtime<Update::eReductions | Update::eHeapAlloc>();
    a.mov(ARG2, TMP_MEM1q);
    a.sub(ARG2, imm(sizeof(UWord)));
    a.mov(ARG1, c_p);
    a.lea(ARG2, x86::qword_ptr(ARG2, -(int)sizeof(ErtsCodeMFA)));
    load_x_reg_array(ARG3);
    a.mov(ARG4, imm(am_breakpoint));
    runtime_call<
            const Export *(*)(Process *, const ErtsCodeMFA *, Eterm *, Eterm),
            call_error_handler>();
    emit_leave_runtime<Update::eReductions | Update::eHeapAlloc>();
    emit_leave_frame();
    a.test(RET, RET);
    a.je(error);
    a.jmp(emit_setup_dispatchable_call(RET));
    a.bind(error);
    {
        a.mov(ARG2, TMP_MEM1q);
        a.jmp(labels[raise_exception]);
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
    a.mov(ARG2, getYRef(0));
    a.mov(ARG3, getXRef(0));
    a.mov(ARG4, getYRef(1));
    a.mov(ARG5, getYRef(2));
    emit_enter_runtime<Update::eHeapAlloc>();
    a.mov(ARG1, c_p);
    runtime_call<void (*)(Process *, ErtsCodeMFA *, Eterm, ErtsTracer, Eterm),
                 return_trace>();
    emit_leave_runtime<Update::eHeapAlloc>();
    emit_deallocate(ArgWord(BEAM_RETURN_TRACE_FRAME_SZ));
    emit_return_do(true);
}
void BeamModuleAssembler::emit_i_call_trace_return() {
    a.mov(ARG2, getYRef(0));
    mov_imm(ARG4, 0);
    a.test(ARG2, imm(_CPMASK));
    a.lea(ARG2, x86::qword_ptr(ARG2, -(Sint)sizeof(ErtsCodeInfo)));
    a.cmovnz(ARG2, ARG4);
    a.mov(ARG3, getYRef(1));
    a.mov(ARG4, getYRef(2));
    emit_enter_runtime<Update::eHeapAlloc>();
    a.mov(ARG1, c_p);
    runtime_call<void (*)(Process *, const ErtsCodeInfo *, Eterm, Eterm),
                 erts_call_trace_return>();
    emit_leave_runtime<Update::eHeapAlloc>();
    emit_deallocate(ArgWord(BEAM_RETURN_CALL_ACC_TRACE_FRAME_SZ));
    emit_return_do(true);
}
void BeamModuleAssembler::emit_i_return_to_trace() {
    UWord frame_size = BEAM_RETURN_TO_TRACE_FRAME_SZ;
#if !defined(NATIVE_ERLANG_STACK)
    frame_size += CP_SIZE;
#endif
    a.mov(ARG2, getYRef(0));
    a.lea(ARG3, x86::qword_ptr(E, frame_size * sizeof(Eterm)));
    emit_enter_runtime<Update::eReductions | Update::eHeapAlloc>();
    a.mov(ARG1, c_p);
    runtime_call<void (*)(Process *, Eterm, Eterm *),
                 beam_jit_return_to_trace>();
    emit_leave_runtime<Update::eReductions | Update::eHeapAlloc>();
    emit_deallocate(ArgWord(BEAM_RETURN_TO_TRACE_FRAME_SZ));
    emit_return_do(true);
}
void BeamModuleAssembler::emit_i_hibernate() {
    emit_enter_runtime<Update::eReductions | Update::eHeap | Update::eStack>();
    a.mov(ARG1, c_p);
    load_x_reg_array(ARG2);
    mov_imm(ARG3, 0);
    runtime_call<void (*)(Process *, Eterm *, int), erts_hibernate>();
    emit_leave_runtime<Update::eReductions | Update::eHeap | Update::eStack>();
    a.and_(x86::dword_ptr(c_p, offsetof(Process, flags)),
           imm(~F_HIBERNATE_SCHED));
    a.mov(getXRef(0), imm(am_ok));
#ifdef NATIVE_ERLANG_STACK
    fragment_call(resolve_fragment(ga->get_dispatch_return()));
#else
    Label next = a.new_label();
    a.lea(ARG3, x86::qword_ptr(next));
    a.jmp(resolve_fragment(ga->get_dispatch_return()));
    a.align(AlignMode::kCode, 8);
    a.bind(next);
#endif
}