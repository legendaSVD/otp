#define ERTS_BEAM_ASM_GLOBAL_WANT_STATIC_DEFS
#include "beam_asm.hpp"
#undef ERTS_BEAM_ASM_GLOBAL_WANT_STATIC_DEFS
using namespace asmjit;
extern "C"
{
#include "bif.h"
#include "beam_common.h"
}
BeamGlobalAssembler::BeamGlobalAssembler(JitAllocator *allocator)
        : BeamAssembler("beam_asm_global") {
    labels.reserve(emitPtrs.size());
    for (auto val : labelNames) {
        std::string name = "global::" + val.second;
        labels[val.first] = a.new_named_label(name.c_str());
    }
    for (auto val : emitPtrs) {
        a.align(AlignMode::kCode, 8);
        a.bind(labels[val.first]);
        (this->*val.second)();
    }
    {
        const void *executable_region;
        void *writable_region;
        BeamAssembler::codegen(allocator, &executable_region, &writable_region);
        VirtMem::flush_instruction_cache((void *)executable_region,
                                         code.code_size());
        VirtMem::protect_jit_memory(VirtMem::ProtectJitAccess::kReadExecute);
    }
#ifndef WIN32
    std::vector<AsmRange> ranges;
    ranges.reserve(emitPtrs.size());
    for (auto val : emitPtrs) {
        ErtsCodePtr start = (ErtsCodePtr)getCode(labels[val.first]);
        ErtsCodePtr stop;
        if (val.first + 1 < emitPtrs.size()) {
            stop = (ErtsCodePtr)getCode(labels[(GlobalLabels)(val.first + 1)]);
        } else {
            stop = (ErtsCodePtr)((char *)getBaseAddress() + code.code_size());
        }
        ranges.push_back(
                {.start = start,
                 .stop = stop,
                 .name = code.label_entry_of(labels[val.first]).name()});
    }
    (void)beamasm_metadata_insert("global",
                                  (ErtsCodePtr)getBaseAddress(),
                                  code.code_size(),
                                  ranges);
#endif
    for (auto val : labelNames) {
        ptrs[val.first] = (fptr)getCode(labels[val.first]);
    }
}
void BeamGlobalAssembler::emit_garbage_collect() {
    Label exiting = a.new_label();
    emit_enter_frame();
    a.sub(ARG3, HTOP);
    a.shr(ARG3, imm(3));
    a.lea(ARG2, x86::qword_ptr(ARG3, -S_RESERVED));
    if (erts_frame_layout == ERTS_FRAME_LAYOUT_RA) {
        a.mov(RET, x86::qword_ptr(x86::rsp));
    } else {
        ASSERT(erts_frame_layout == ERTS_FRAME_LAYOUT_FP_RA);
        a.mov(RET, x86::qword_ptr(x86::rsp, 8));
    }
    a.mov(x86::qword_ptr(c_p, offsetof(Process, i)), RET);
    emit_enter_runtime<Update::eStack | Update::eHeap>();
    a.mov(ARG1, c_p);
    load_x_reg_array(ARG3);
    a.mov(ARG5d, FCALLS);
    runtime_call<int (*)(Process *, Uint, Eterm *, int, int),
                 erts_garbage_collect_nobump>();
    a.sub(FCALLS, RETd);
    emit_leave_runtime<Update::eStack | Update::eHeap>();
#ifdef WIN32
    a.mov(ARG1d, x86::dword_ptr(c_p, offsetof(Process, state.value)));
#else
    a.mov(ARG1d, x86::dword_ptr(c_p, offsetof(Process, state.counter)));
#endif
    a.test(ARG1d, imm(ERTS_PSFLG_EXITING));
    a.short_().jne(exiting);
    emit_leave_frame();
    a.ret();
    a.bind(exiting);
    emit_unwind_frame();
    a.jmp(labels[do_schedule]);
}
void BeamGlobalAssembler::emit_bif_export_trap() {
    a.mov(RET, x86::qword_ptr(c_p, offsetof(Process, current)));
    a.sub(RET, imm(offsetof(Export, info.mfa)));
    emit_leave_frame();
    a.jmp(emit_setup_dispatchable_call(RET));
}
void BeamGlobalAssembler::emit_export_trampoline() {
    Label call_bif = a.new_label(), error_handler = a.new_label();
    a.mov(ARG1, x86::qword_ptr(RET, offsetof(Export, trampoline.common.op)));
    a.cmp(ARG1, imm(op_i_generic_breakpoint));
    a.je(labels[generic_bp_global]);
    a.cmp(ARG1, imm(op_call_bif_W));
    a.je(call_bif);
    a.cmp(ARG1, imm(op_call_error_handler));
    a.je(error_handler);
    comment("Unexpected export trampoline op");
    a.ud2();
    a.bind(call_bif);
    {
        ssize_t func_offset = offsetof(Export, trampoline.bif.address);
        a.lea(ARG2, x86::qword_ptr(RET, offsetof(Export, info.mfa)));
        a.mov(ARG3, x86::qword_ptr(c_p, offsetof(Process, i)));
        a.mov(ARG4, x86::qword_ptr(RET, func_offset));
        emit_enter_frame();
        a.jmp(labels[call_bif_shared]);
    }
    a.bind(error_handler);
    {
        Label error;
#ifdef NATIVE_ERLANG_STACK
        error = labels[raise_exception];
#else
        error = a.new_label();
#endif
        a.lea(ARG2, x86::qword_ptr(RET, offsetof(Export, info.mfa)));
        a.mov(TMP_MEM1q, ARG2);
        emit_enter_frame();
        emit_enter_runtime<Update::eReductions | Update::eHeapAlloc>();
        a.mov(ARG1, c_p);
        load_x_reg_array(ARG3);
        mov_imm(ARG4, am_undefined_function);
        runtime_call<
                const Export
                        *(*)(Process *, const ErtsCodeMFA *, Eterm *, Eterm),
                call_error_handler>();
        emit_leave_runtime<Update::eReductions | Update::eHeapAlloc>();
        emit_leave_frame();
        a.mov(ARG4, TMP_MEM1q);
        a.test(RET, RET);
        a.je(error);
        a.jmp(emit_setup_dispatchable_call(RET));
#ifndef NATIVE_ERLANG_STACK
        a.bind(error);
        {
            a.push(getCPRef());
            a.mov(getCPRef(), imm(NIL));
            a.jmp(labels[raise_exception]);
        }
#endif
    }
}
void BeamModuleAssembler::emit_raise_exception() {
    safe_fragment_call(ga->get_raise_exception_null_exp());
    last_error_offset = a.offset();
}
void BeamModuleAssembler::emit_raise_exception(const ErtsCodeMFA *exp) {
    mov_imm(ARG4, exp);
    safe_fragment_call(ga->get_raise_exception());
    last_error_offset = a.offset();
}
void BeamModuleAssembler::emit_raise_exception(Label I,
                                               const ErtsCodeMFA *exp) {
    a.lea(ARG2, x86::qword_ptr(I));
    emit_raise_exception(ARG2, exp);
}
void BeamModuleAssembler::emit_raise_exception(x86::Gp I,
                                               const ErtsCodeMFA *exp) {
    if (I != ARG2) {
        a.mov(ARG2, I);
    }
    mov_imm(ARG4, exp);
#ifdef NATIVE_ERLANG_STACK
    a.push(ARG2);
    if (erts_frame_layout == ERTS_FRAME_LAYOUT_FP_RA) {
#    ifdef ERLANG_FRAME_POINTERS
        a.push(frame_pointer);
#    endif
    } else {
        ASSERT(erts_frame_layout == ERTS_FRAME_LAYOUT_RA);
    }
#endif
    a.jmp(resolve_fragment(ga->get_raise_exception_shared()));
}
void BeamGlobalAssembler::emit_process_exit() {
    emit_enter_runtime<Update::eHeapAlloc | Update::eReductions>();
    a.mov(ARG1, c_p);
    mov_imm(ARG2, 0);
    mov_imm(ARG4, 0);
    load_x_reg_array(ARG3);
    runtime_call<ErtsCodePtr (*)(Process *,
                                 ErtsCodePtr,
                                 Eterm *,
                                 const ErtsCodeMFA *),
                 ::handle_error>();
    emit_leave_runtime<Update::eHeapAlloc | Update::eReductions>();
    a.test(RET, RET);
    a.je(labels[do_schedule]);
    comment("End of process");
    a.ud2();
}
void BeamGlobalAssembler::emit_raise_exception_null_exp() {
    mov_imm(ARG4, 0);
    a.jmp(labels[raise_exception]);
}
void BeamGlobalAssembler::emit_raise_exception() {
    a.pop(ARG2);
    a.and_(ARG2, imm(~_CPMASK));
#ifdef NATIVE_ERLANG_STACK
    a.push(ARG2);
    if (erts_frame_layout == ERTS_FRAME_LAYOUT_FP_RA) {
#    ifdef ERLANG_FRAME_POINTERS
        a.push(frame_pointer);
#    endif
    } else {
        ASSERT(erts_frame_layout == ERTS_FRAME_LAYOUT_RA);
    }
#endif
    a.jmp(labels[raise_exception_shared]);
}
void BeamGlobalAssembler::emit_raise_exception_shared() {
    Label crash = a.new_label();
    emit_enter_runtime<Update::eHeapAlloc>();
    a.test(ARG2d, imm(_CPMASK));
    a.short_().jne(crash);
    a.mov(ARG1, c_p);
    load_x_reg_array(ARG3);
    runtime_call<ErtsCodePtr (*)(Process *,
                                 ErtsCodePtr,
                                 Eterm *,
                                 const ErtsCodeMFA *),
                 ::handle_error>();
    emit_leave_runtime<Update::eHeapAlloc>();
    a.test(RET, RET);
    a.je(labels[do_schedule]);
    a.jmp(RET);
    a.bind(crash);
    comment("Error address is not a CP or NULL or ARG2 and ARG4 are unset");
    a.ud2();
}
void BeamModuleAssembler::emit_proc_lc_unrequire(void) {
#ifdef ERTS_ENABLE_LOCK_CHECK
    emit_assert_runtime_stack();
    a.mov(ARG1, c_p);
    a.mov(ARG2, imm(ERTS_PROC_LOCK_MAIN));
    a.mov(TMP_MEM1q, RET);
    runtime_call<void (*)(Process *, ErtsProcLocks),
                 erts_proc_lc_unrequire_lock>();
    a.mov(RET, TMP_MEM1q);
#endif
}
void BeamModuleAssembler::emit_proc_lc_require(void) {
#ifdef ERTS_ENABLE_LOCK_CHECK
    emit_assert_runtime_stack();
    a.mov(ARG1, c_p);
    a.mov(ARG2, imm(ERTS_PROC_LOCK_MAIN));
    a.mov(TMP_MEM1q, RET);
    runtime_call<void (*)(Process *, ErtsProcLocks, const char *, unsigned int),
                 erts_proc_lc_require_lock>();
    a.mov(RET, TMP_MEM1q);
#endif
}
extern "C"
{
    void ERTS_NOINLINE __jit_debug_register_code(void);
    void ERTS_NOINLINE __jit_debug_register_code(void) {
    }
}