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
        ranges.push_back(AsmRange{start,
                                  stop,
                                  code.label_entry_of(labels[val.first]).name(),
                                  {}});
    }
    (void)beamasm_metadata_insert("global",
                                  (ErtsCodePtr)getBaseAddress(),
                                  code.code_size(),
                                  ranges);
    for (auto val : labelNames) {
        ptrs[val.first] = (fptr)getCode(labels[val.first]);
    }
}
void BeamGlobalAssembler::emit_garbage_collect() {
    emit_enter_runtime_frame();
    a.sub(ARG2, ARG3, HTOP);
    a.lsr(ARG2, ARG2, imm(3));
    a.sub(ARG2, ARG2, imm(S_RESERVED));
    a.str(a64::x30, a64::Mem(c_p, offsetof(Process, i)));
    emit_enter_runtime<Update::eStack | Update::eHeap | Update::eXRegs>();
    a.mov(ARG1, c_p);
    load_x_reg_array(ARG3);
    a.mov(ARG5.w(), FCALLS);
    runtime_call<int (*)(Process *, Uint, Eterm *, int, int),
                 erts_garbage_collect_nobump>();
    a.sub(FCALLS, FCALLS, ARG1.w());
    emit_leave_runtime<Update::eStack | Update::eHeap | Update::eXRegs>();
    emit_leave_runtime_frame();
    a.ldr(TMP1.w(), a64::Mem(c_p, offsetof(Process, state.value)));
    a.tst(TMP1, imm(ERTS_PSFLG_EXITING));
    a.b_ne(labels[do_schedule]);
    a.ret(a64::x30);
}
void BeamGlobalAssembler::emit_bif_export_trap() {
    int export_offset = offsetof(Export, info.mfa);
    a.ldr(ARG1, a64::Mem(c_p, offsetof(Process, current)));
    a.sub(ARG1, ARG1, export_offset);
    emit_leave_erlang_frame();
    branch(emit_setup_dispatchable_call(ARG1));
}
void BeamGlobalAssembler::emit_export_trampoline() {
    Label call_bif = a.new_label(), error_handler = a.new_label();
    a.ldr(TMP1, a64::Mem(ARG1, offsetof(Export, trampoline.common.op)));
    a.cmp(TMP1, imm(op_i_generic_breakpoint));
    a.b_eq(labels[generic_bp_global]);
    a.cmp(TMP1, imm(op_call_bif_W));
    a.b_eq(call_bif);
    a.cmp(TMP1, imm(op_call_error_handler));
    a.b_eq(error_handler);
    a.udf(0xffff);
    a.bind(call_bif);
    {
        ssize_t func_offset = offsetof(Export, trampoline.bif.address);
        lea(ARG2, a64::Mem(ARG1, offsetof(Export, info.mfa)));
        a.ldr(ARG3, a64::Mem(c_p, offsetof(Process, i)));
        a.ldr(ARG4, a64::Mem(ARG1, func_offset));
        emit_enter_erlang_frame();
        a.b(labels[call_bif_shared]);
    }
    a.bind(error_handler);
    {
        lea(ARG2, a64::Mem(ARG1, offsetof(Export, info.mfa)));
        a.str(ARG2, TMP_MEM1q);
        emit_enter_runtime_frame();
        emit_enter_runtime<Update::eReductions | Update::eHeapAlloc |
                           Update::eXRegs>();
        a.mov(ARG1, c_p);
        load_x_reg_array(ARG3);
        mov_imm(ARG4, am_undefined_function);
        runtime_call<
                const Export
                        *(*)(Process *, const ErtsCodeMFA *, Eterm *, Eterm),
                call_error_handler>();
        emit_leave_runtime<Update::eReductions | Update::eHeapAlloc |
                           Update::eXRegs>();
        emit_leave_runtime_frame();
        a.ldr(ARG4, TMP_MEM1q);
        a.cbz(ARG1, labels[raise_exception]);
        branch(emit_setup_dispatchable_call(ARG1));
    }
}
void BeamModuleAssembler::emit_raise_exception() {
    emit_raise_exception(nullptr);
}
void BeamModuleAssembler::emit_raise_exception(const ErtsCodeMFA *exp) {
    if (exp) {
        a.ldr(ARG4, embed_constant(exp, disp32K));
        fragment_call(ga->get_raise_exception());
    } else {
        fragment_call(ga->get_raise_exception_null_exp());
    }
    last_error_offset = a.offset();
}
void BeamModuleAssembler::emit_raise_exception(Label I,
                                               const ErtsCodeMFA *exp) {
    a.adr(ARG2, I);
    if (exp) {
        a.ldr(ARG4, embed_constant(exp, disp32K));
        a.b(resolve_fragment(ga->get_raise_exception_shared(), disp128MB));
    } else {
        a.b(resolve_fragment(ga->get_raise_exception_null_exp(), disp128MB));
    }
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
    a.cbz(ARG1, labels[do_schedule]);
    a.udf(0xdead);
}
void BeamGlobalAssembler::emit_raise_exception_null_exp() {
    a.mov(ARG4, ZERO);
    a.mov(ARG2, a64::x30);
    a.b(labels[raise_exception_shared]);
}
void BeamGlobalAssembler::emit_raise_exception() {
    a.mov(ARG2, a64::x30);
    a.b(labels[raise_exception_shared]);
}
void BeamGlobalAssembler::emit_raise_exception_shared() {
    Label crash = a.new_label();
    a.str(ARG2, a64::Mem(E, -8).pre());
    emit_enter_runtime<Update::eHeapAlloc | Update::eXRegs>();
    a.tst(ARG2, imm(_CPMASK));
    a.b_ne(crash);
    a.mov(ARG1, c_p);
    load_x_reg_array(ARG3);
    runtime_call<ErtsCodePtr (*)(Process *,
                                 ErtsCodePtr,
                                 Eterm *,
                                 const ErtsCodeMFA *),
                 ::handle_error>();
    emit_leave_runtime<Update::eHeapAlloc | Update::eXRegs>();
    a.cbz(ARG1, labels[do_schedule]);
    a.br(ARG1);
    a.bind(crash);
    a.udf(0xbad);
}
void BeamModuleAssembler::emit_proc_lc_unrequire(void) {
#ifdef ERTS_ENABLE_LOCK_CHECK
    a.mov(ARG1, c_p);
    mov_imm(ARG2, ERTS_PROC_LOCK_MAIN);
    runtime_call<void (*)(Process *, ErtsProcLocks),
                 erts_proc_lc_unrequire_lock>();
#endif
}
void BeamModuleAssembler::emit_proc_lc_require(void) {
#ifdef ERTS_ENABLE_LOCK_CHECK
    a.mov(ARG1, c_p);
    mov_imm(ARG2, ERTS_PROC_LOCK_MAIN);
    runtime_call<void (*)(Process *, ErtsProcLocks, const char *, unsigned int),
                 erts_proc_lc_require_lock>();
#endif
}
extern "C"
{
    void ERTS_NOINLINE __jit_debug_register_code(void);
    void ERTS_NOINLINE __jit_debug_register_code(void) {
    }
}