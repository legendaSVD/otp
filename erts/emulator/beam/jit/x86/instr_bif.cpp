#include "beam_asm.hpp"
extern "C"
{
#include "beam_common.h"
#include "code_ix.h"
#include "erl_bif_table.h"
#include "erl_nfunc_sched.h"
#include "bif.h"
#include "erl_msacc.h"
}
#if defined(ERTS_CCONV_DEBUG)
static Eterm ERTS_CCONV_JIT bif_cconv_trampoline(Process *c_p,
                                                 Eterm *reg,
                                                 ErtsCodePtr I,
                                                 ErtsBifFunc func) {
    return func(c_p, reg, I);
}
#endif
void BeamGlobalAssembler::emit_i_bif_guard_shared() {
    emit_enter_frame();
    emit_enter_runtime<Update::eReductions>();
    a.mov(ARG1, c_p);
    mov_imm(ARG3, 0);
#if defined(ERTS_CCONV_DEBUG)
    runtime_call<Eterm(ERTS_CCONV_JIT
                               *)(Process *, Eterm *, ErtsCodePtr, ErtsBifFunc),
                 bif_cconv_trampoline>();
#else
    dynamic_runtime_call<3>(ARG4);
#endif
    emit_leave_runtime<Update::eReductions>();
    emit_leave_frame();
    emit_test_the_non_value(RET);
    a.ret();
}
void BeamGlobalAssembler::emit_i_bif_body_shared() {
    Label error = a.new_label();
    emit_enter_frame();
    emit_enter_runtime<Update::eReductions>();
    a.mov(TMP_MEM1q, ARG2);
    a.mov(TMP_MEM2q, ARG4);
    a.mov(ARG1, c_p);
    mov_imm(ARG3, 0);
#if defined(ERTS_CCONV_DEBUG)
    runtime_call<Eterm(ERTS_CCONV_JIT
                               *)(Process *, Eterm *, ErtsCodePtr, ErtsBifFunc),
                 bif_cconv_trampoline>();
#else
    dynamic_runtime_call<3>(ARG4);
#endif
    emit_test_the_non_value(RET);
    a.short_().je(error);
    emit_leave_runtime<Update::eReductions>();
    emit_leave_frame();
    a.ret();
    a.bind(error);
    {
        a.mov(ARG2, TMP_MEM1q);
        for (int i = 0; i < 3; i++) {
            a.mov(ARG1, x86::qword_ptr(ARG2, i * sizeof(Eterm)));
            a.mov(getXRef(i), ARG1);
        }
        a.mov(ARG1, TMP_MEM2q);
        runtime_call<ErtsCodeMFA *(*)(void *), ubif2mfa>();
        emit_leave_runtime<Update::eReductions>();
        emit_leave_frame();
        a.mov(ARG4, RET);
        a.jmp(labels[raise_exception]);
    }
}
void BeamModuleAssembler::emit_setup_guard_bif(const std::vector<ArgVal> &args,
                                               const ArgWord &Bif) {
    bool is_contiguous_mem = false;
    ASSERT(args.size() > 0 && args.size() <= 3);
    is_contiguous_mem = args.size() && args[0].isRegister();
    for (size_t i = 1; i < args.size() && is_contiguous_mem; i++) {
        const ArgSource &curr = args[i], &prev = args[i - 1];
        is_contiguous_mem = ArgVal::memory_relation(prev, curr) ==
                            ArgVal::Relation::consecutive;
    }
    if (is_contiguous_mem) {
        a.lea(ARG2, getArgRef(args[0]));
    } else {
        a.lea(ARG2, TMP_MEM3q);
        for (size_t i = 0; i < args.size(); i++) {
            mov_arg(x86::qword_ptr(ARG2, i * sizeof(Eterm)), args[i]);
        }
    }
    if (logger.file()) {
        ErtsCodeMFA *mfa = ubif2mfa((void *)Bif.get());
        if (mfa) {
            comment("UBIF: %T/%d", mfa->function, mfa->arity);
        }
    }
    mov_arg(ARG4, Bif);
}
void BeamModuleAssembler::emit_i_bif1(const ArgSource &Src1,
                                      const ArgLabel &Fail,
                                      const ArgWord &Bif,
                                      const ArgRegister &Dst) {
    emit_setup_guard_bif({Src1}, Bif);
    if (Fail.get() != 0) {
        safe_fragment_call(ga->get_i_bif_guard_shared());
        a.je(resolve_beam_label(Fail));
    } else {
        safe_fragment_call(ga->get_i_bif_body_shared());
    }
    mov_arg(Dst, RET);
}
void BeamModuleAssembler::emit_i_bif2(const ArgSource &Src1,
                                      const ArgSource &Src2,
                                      const ArgLabel &Fail,
                                      const ArgWord &Bif,
                                      const ArgRegister &Dst) {
    emit_setup_guard_bif({Src1, Src2}, Bif);
    if (Fail.get() != 0) {
        safe_fragment_call(ga->get_i_bif_guard_shared());
        a.je(resolve_beam_label(Fail));
    } else {
        safe_fragment_call(ga->get_i_bif_body_shared());
    }
    mov_arg(Dst, RET);
}
void BeamModuleAssembler::emit_i_bif3(const ArgSource &Src1,
                                      const ArgSource &Src2,
                                      const ArgSource &Src3,
                                      const ArgLabel &Fail,
                                      const ArgWord &Bif,
                                      const ArgRegister &Dst) {
    emit_setup_guard_bif({Src1, Src2, Src3}, Bif);
    if (Fail.get() != 0) {
        safe_fragment_call(ga->get_i_bif_guard_shared());
        a.je(resolve_beam_label(Fail));
    } else {
        safe_fragment_call(ga->get_i_bif_body_shared());
    }
    mov_arg(Dst, RET);
}
void BeamModuleAssembler::emit_nofail_bif1(const ArgSource &Src1,
                                           const ArgWord &Bif,
                                           const ArgRegister &Dst) {
    emit_setup_guard_bif({Src1}, Bif);
    safe_fragment_call(ga->get_i_bif_guard_shared());
    mov_arg(Dst, RET);
}
void BeamModuleAssembler::emit_nofail_bif2(const ArgSource &Src1,
                                           const ArgSource &Src2,
                                           const ArgWord &Bif,
                                           const ArgRegister &Dst) {
    emit_setup_guard_bif({Src1, Src2}, Bif);
    safe_fragment_call(ga->get_i_bif_guard_shared());
    mov_arg(Dst, RET);
}
void BeamModuleAssembler::emit_i_length_setup(const ArgLabel &Fail,
                                              const ArgWord &Live,
                                              const ArgSource &Src) {
    x86::Mem trap_state;
    ERTS_CT_ASSERT(ERTS_X_REGS_ALLOCATED - MAX_REG >= 3);
    ASSERT(Live.get() <= MAX_REG);
    trap_state = getXRef(Live.get());
    mov_arg(trap_state, Src);
    a.mov(trap_state.clone_adjusted(1 * sizeof(Eterm)), imm(make_small(0)));
    if (Fail.get() == 0) {
        x86::Mem original_argument;
        original_argument = trap_state.clone_adjusted(2 * sizeof(Eterm));
        mov_arg(original_argument, Src);
    }
}
x86::Mem BeamGlobalAssembler::emit_i_length_common(Label fail, int state_size) {
    Label trap = a.new_label();
    x86::Mem trap_state;
    ASSERT(state_size >= 2 && state_size <= ERTS_X_REGS_ALLOCATED - MAX_REG);
    trap_state = getXRef(0);
    trap_state.set_index(ARG2, 3);
    emit_enter_frame();
    a.mov(TMP_MEM1q, ARG2);
    a.mov(TMP_MEM2q, ARG3);
    emit_enter_runtime<Update::eReductions>();
    a.mov(ARG1, c_p);
    a.lea(ARG2, trap_state);
    runtime_call<Eterm (*)(Process *, Eterm *), erts_trapping_length_1>();
    emit_leave_runtime<Update::eReductions>();
    emit_leave_frame();
    emit_test_the_non_value(RET);
    a.short_().je(trap);
    a.ret();
    a.bind(trap);
    {
        a.mov(ARG2, TMP_MEM1q);
        a.mov(ARG3, TMP_MEM2q);
        a.cmp(x86::qword_ptr(c_p, offsetof(Process, freason)), imm(TRAP));
        a.jne(fail);
        a.add(ARG2, imm(state_size));
        a.add(x86::rsp, imm(sizeof(UWord)));
        a.mov(x86::qword_ptr(c_p, offsetof(Process, current)), imm(0));
        a.mov(x86::byte_ptr(c_p, offsetof(Process, arity)), ARG2.r8());
        a.jmp(labels[context_switch_simplified]);
    }
    return trap_state;
}
void BeamGlobalAssembler::emit_i_length_body_shared() {
    Label error = a.new_label();
    x86::Mem trap_state;
    trap_state = emit_i_length_common(error, 3);
    a.bind(error);
    {
        static const ErtsCodeMFA bif_mfa = {am_erlang, am_length, 1};
        a.mov(ARG1, trap_state.clone_adjusted(2 * sizeof(Eterm)));
        a.mov(getXRef(0), ARG1);
        a.mov(ARG4, imm(&bif_mfa));
        a.jmp(labels[raise_exception]);
    }
}
void BeamGlobalAssembler::emit_i_length_guard_shared() {
    Label error = a.new_label();
    emit_i_length_common(error, 2);
    a.bind(error);
    {
        a.sub(RET, RET);
        a.ret();
    }
}
void BeamModuleAssembler::emit_i_length(const ArgLabel &Fail,
                                        const ArgWord &Live,
                                        const ArgRegister &Dst) {
    Label entry = a.new_label();
    align_erlang_cp();
    a.bind(entry);
    mov_arg(ARG2, Live);
    a.lea(ARG3, x86::qword_ptr(entry));
    if (Fail.get() != 0) {
        safe_fragment_call(ga->get_i_length_guard_shared());
        a.je(resolve_beam_label(Fail));
    } else {
        safe_fragment_call(ga->get_i_length_body_shared());
    }
    mov_arg(Dst, RET);
}
#if defined(DEBUG) || defined(ERTS_ENABLE_LOCK_CHECK)
static Eterm debug_call_light_bif(Process *c_p,
                                  Eterm *reg,
                                  ErtsCodePtr I,
                                  ErtsBifFunc vbf) {
    Eterm result;
    ERTS_ASSERT_TRACER_REFS(&c_p->common);
    ERTS_UNREQ_PROC_MAIN_LOCK(c_p);
    {
        ERTS_CHK_MBUF_SZ(c_p);
        ASSERT(!ERTS_PROC_IS_EXITING(c_p));
        result = vbf(c_p, reg, I);
        ASSERT(!ERTS_PROC_IS_EXITING(c_p) || is_non_value(result));
        ERTS_CHK_MBUF_SZ(c_p);
        ERTS_VERIFY_UNUSED_TEMP_ALLOC(c_p);
        ERTS_HOLE_CHECK(c_p);
    }
    PROCESS_MAIN_CHK_LOCKS(c_p);
    ERTS_REQ_PROC_MAIN_LOCK(c_p);
    ERTS_ASSERT_TRACER_REFS(&c_p->common);
    return result;
}
#endif
void BeamGlobalAssembler::emit_call_light_bif_shared() {
    x86::Mem entry_mem = TMP_MEM1q, export_mem = TMP_MEM2q,
             mbuf_mem = TMP_MEM3q;
    Label trace = a.new_label(), yield = a.new_label();
    emit_enter_frame();
    a.mov(ARG1, x86::qword_ptr(c_p, offsetof(Process, mbuf)));
    a.mov(entry_mem, ARG3);
    a.mov(export_mem, ARG4);
    a.mov(mbuf_mem, ARG1);
    a.cmp(x86::dword_ptr(ARG4, offsetof(Export, is_bif_traced)), imm(0));
    a.jne(trace);
    a.cmp(active_code_ix, imm(ERTS_SAVE_CALLS_CODE_IX));
    a.je(trace);
    a.dec(FCALLS);
    a.jle(yield);
    {
        Label check_bif_return = a.new_label(),
              gc_after_bif_call = a.new_label();
        emit_enter_runtime<Update::eReductions | Update::eStack |
                           Update::eHeap>();
#ifdef ERTS_MSACC_EXTENDED_STATES
        {
            Label skip_msacc = a.new_label();
            a.cmp(erts_msacc_cache, imm(0));
            a.short_().je(skip_msacc);
            a.mov(TMP_MEM4q, ARG3);
            a.mov(ARG1, erts_msacc_cache);
            a.mov(ARG2,
                  x86::qword_ptr(ARG4, offsetof(Export, info.mfa.module)));
            a.mov(ARG3, RET);
            runtime_call<const void *(*)(ErtsMsAcc *, Eterm, const void *),
                         erts_msacc_set_bif_state>();
            a.mov(ARG3, TMP_MEM4q);
            a.bind(skip_msacc);
        }
#endif
        {
            a.mov(ARG1, c_p);
            load_x_reg_array(ARG2);
            a.mov(ARG4, RET);
#if defined(DEBUG) || defined(ERTS_ENABLE_LOCK_CHECK)
            runtime_call<
                    Eterm (*)(Process *, Eterm *, ErtsCodePtr, ErtsBifFunc),
                    debug_call_light_bif>();
#else
#    if defined(ERTS_CCONV_DEBUG)
            runtime_call<Eterm(ERTS_CCONV_JIT *)(Process *,
                                                 Eterm *,
                                                 ErtsCodePtr,
                                                 ErtsBifFunc),
                         bif_cconv_trampoline>();
#    else
            dynamic_runtime_call<3>(ARG4);
#    endif
#endif
        }
#ifdef ERTS_MSACC_EXTENDED_STATES
        {
            Label skip_msacc = a.new_label();
            a.cmp(erts_msacc_cache, imm(0));
            a.short_().je(skip_msacc);
            {
                a.mov(TMP_MEM4q, RET);
                a.lea(ARG1, erts_msacc_cache);
                runtime_call<void (*)(ErtsMsAcc **), erts_msacc_update_cache>();
                a.mov(RET, TMP_MEM4q);
                a.cmp(erts_msacc_cache, imm(0));
                a.short_().je(skip_msacc);
                a.mov(ARG1, erts_msacc_cache);
                a.mov(ARG2, imm(ERTS_MSACC_STATE_EMULATOR));
                a.mov(ARG3, imm(1));
                runtime_call<void (*)(ErtsMsAcc *, Uint, int),
                             erts_msacc_set_state_m__>();
                a.mov(RET, TMP_MEM4q);
            }
            a.bind(skip_msacc);
        }
#endif
        emit_leave_runtime<Update::eReductions | Update::eStack |
                           Update::eHeap | Update::eCodeIndex>();
        {
            a.test(x86::dword_ptr(c_p, offsetof(Process, flags)),
                   imm(F_FORCE_GC | F_DISABLE_GC));
            a.jne(gc_after_bif_call);
            a.mov(ARG1, x86::qword_ptr(c_p, offsetof(Process, bin_vheap_sz)));
            a.cmp(x86::qword_ptr(c_p, offsetof(Process, off_heap.overhead)),
                  ARG1);
            a.ja(gc_after_bif_call);
            a.mov(ARG2, x86::qword_ptr(c_p, offsetof(Process, mbuf_sz)));
            a.lea(ARG1, x86::qword_ptr(HTOP, ARG2, 0, 3));
            a.cmp(E, ARG1);
            a.jl(gc_after_bif_call);
        }
        a.bind(check_bif_return);
        {
            Label trap = a.new_label(), error = a.new_label();
            emit_test_the_non_value(RET);
            a.short_().je(trap);
            a.mov(getXRef(0), RET);
            emit_leave_frame();
            a.ret();
            a.bind(trap);
            {
                a.cmp(x86::qword_ptr(c_p, offsetof(Process, freason)),
                      imm(TRAP));
                a.short_().jne(error);
#if !defined(NATIVE_ERLANG_STACK)
                a.pop(getCPRef());
#endif
                a.mov(ARG3, x86::qword_ptr(c_p, offsetof(Process, i)));
                a.jmp(labels[context_switch_simplified]);
            }
            a.bind(error);
            {
                a.mov(ARG2, entry_mem);
                a.mov(ARG4, export_mem);
                a.add(ARG4, imm(offsetof(Export, info.mfa)));
#if !defined(NATIVE_ERLANG_STACK)
                emit_unwind_frame();
#endif
                if (erts_frame_layout == ERTS_FRAME_LAYOUT_RA) {
                    a.mov(x86::qword_ptr(E), ARG2);
                } else {
                    ASSERT(erts_frame_layout == ERTS_FRAME_LAYOUT_FP_RA);
                    a.mov(x86::qword_ptr(E, 8), ARG2);
                }
                a.jmp(labels[raise_exception_shared]);
            }
        }
        a.bind(gc_after_bif_call);
        {
            a.mov(ARG2, mbuf_mem);
            a.mov(ARG5, export_mem);
            a.movzx(ARG5d,
                    x86::byte_ptr(ARG5, offsetof(Export, info.mfa.arity)));
            emit_enter_runtime<Update::eReductions | Update::eStack |
                               Update::eHeap>();
            a.mov(ARG1, c_p);
            a.mov(ARG3, RET);
            load_x_reg_array(ARG4);
            runtime_call<Eterm (*)(Process *,
                                   ErlHeapFragment *,
                                   Eterm,
                                   Eterm *,
                                   Uint),
                         erts_gc_after_bif_call_lhf>();
            emit_leave_runtime<Update::eReductions | Update::eStack |
                               Update::eHeap>();
            a.jmp(check_bif_return);
        }
    }
    a.bind(trace);
    {
        emit_leave_frame();
#if !defined(NATIVE_ERLANG_STACK)
        a.pop(getCPRef());
#endif
        x86::Mem destination = emit_setup_dispatchable_call(ARG4);
        a.jmp(destination);
    }
    a.bind(yield);
    {
        a.movzx(ARG2d, x86::byte_ptr(ARG4, offsetof(Export, info.mfa.arity)));
        a.lea(ARG4, x86::qword_ptr(ARG4, offsetof(Export, info.mfa)));
        a.mov(x86::byte_ptr(c_p, offsetof(Process, arity)), ARG2.r8());
        a.mov(x86::qword_ptr(c_p, offsetof(Process, current)), ARG4);
        emit_unwind_frame();
        a.jmp(labels[context_switch_simplified]);
    }
}
void BeamModuleAssembler::emit_call_light_bif(const ArgWord &Bif,
                                              const ArgExport &Exp) {
    Label entry = a.new_label();
    align_erlang_cp();
    a.bind(entry);
    mov_arg(ARG4, Exp);
    a.mov(RET, imm(Bif.get()));
    a.lea(ARG3, x86::qword_ptr(entry));
    if (logger.file()) {
        BeamFile_ImportEntry *e = &beam->imports.entries[Exp.get()];
        comment("BIF: %T:%T/%d", e->module, e->function, e->arity);
    }
    fragment_call(ga->get_call_light_bif_shared());
}
void BeamModuleAssembler::emit_send() {
    Label entry = a.new_label();
    align_erlang_cp();
    a.bind(entry);
    a.mov(ARG4, imm(BIF_TRAP_EXPORT(BIF_send_2)));
    a.mov(RET, imm(send_2));
    a.lea(ARG3, x86::qword_ptr(entry));
    fragment_call(ga->get_call_light_bif_shared());
}
void BeamModuleAssembler::emit_nif_start() {
}
void BeamGlobalAssembler::emit_bif_nif_epilogue(void) {
    Label check_trap = a.new_label(), trap = a.new_label(),
          error = a.new_label();
#ifdef ERTS_MSACC_EXTENDED_STATES
    {
        Label skip_msacc = a.new_label();
        a.cmp(erts_msacc_cache, 0);
        a.short_().je(skip_msacc);
        a.mov(TMP_MEM1q, RET);
        a.mov(ARG1, erts_msacc_cache);
        a.mov(ARG2, imm(ERTS_MSACC_STATE_EMULATOR));
        a.mov(ARG3, imm(1));
        runtime_call<void (*)(ErtsMsAcc *, Uint, int),
                     erts_msacc_set_state_m__>();
        a.mov(RET, TMP_MEM1q);
        a.bind(skip_msacc);
    }
#endif
    emit_leave_runtime<Update::eReductions | Update::eStack | Update::eHeap |
                       Update::eCodeIndex>();
    emit_test_the_non_value(RET);
    a.short_().je(check_trap);
    comment("Do return and dispatch to it");
    a.mov(getXRef(0), RET);
    emit_leave_frame();
#ifdef NATIVE_ERLANG_STACK
    if (erts_alcu_enable_code_atags) {
        a.mov(RET, x86::qword_ptr(E));
        a.mov(x86::qword_ptr(c_p, offsetof(Process, i)), RET);
    }
    a.ret();
#else
    a.mov(RET, getCPRef());
    a.mov(getCPRef(), imm(NIL));
    if (erts_alcu_enable_code_atags) {
        a.mov(x86::qword_ptr(c_p, offsetof(Process, i)), RET);
    }
    a.jmp(RET);
#endif
    a.bind(check_trap);
    a.cmp(x86::qword_ptr(c_p, offsetof(Process, freason)), imm(TRAP));
    a.jne(error);
    {
        comment("yield");
        comment("test trap to hibernate");
        a.mov(ARG1d, x86::dword_ptr(c_p, offsetof(Process, flags)));
        a.mov(ARG2d, ARG1d);
        a.and_(ARG2d, imm(F_HIBERNATE_SCHED));
        a.short_().je(trap);
        comment("do hibernate trap");
        a.and_(ARG1d, imm(~F_HIBERNATE_SCHED));
        a.mov(x86::dword_ptr(c_p, offsetof(Process, flags)), ARG1d);
        a.jmp(labels[do_schedule]);
    }
    a.bind(trap);
    {
        comment("do normal trap");
        a.mov(ARG3, x86::qword_ptr(c_p, offsetof(Process, i)));
        a.jmp(labels[context_switch_simplified]);
    }
    a.bind(error);
    {
        a.mov(ARG2, E);
        emit_enter_runtime<Update::eStack>();
        a.mov(ARG1, c_p);
        runtime_call<ErtsCodePtr (*)(const Process *, const Eterm *),
                     erts_printable_return_address>();
        emit_leave_runtime<Update::eStack>();
        a.mov(ARG2, RET);
        a.mov(ARG4, x86::qword_ptr(c_p, offsetof(Process, current)));
        a.jmp(labels[raise_exception_shared]);
    }
}
void BeamGlobalAssembler::emit_call_bif_shared(void) {
    a.mov(x86::qword_ptr(c_p, offsetof(Process, current)), ARG2);
    a.movzx(ARG5d, x86::byte_ptr(ARG2, offsetof(ErtsCodeMFA, arity)));
    a.mov(x86::byte_ptr(c_p, offsetof(Process, arity)), ARG5.r8());
    a.mov(x86::qword_ptr(c_p, offsetof(Process, i)), ARG3);
    emit_enter_runtime<Update::eReductions | Update::eStack | Update::eHeap>();
#ifdef ERTS_MSACC_EXTENDED_STATES
    {
        Label skip_msacc = a.new_label();
        a.cmp(erts_msacc_cache, 0);
        a.short_().je(skip_msacc);
        a.mov(TMP_MEM1q, ARG3);
        a.mov(TMP_MEM2q, ARG5);
        a.mov(ARG1, erts_msacc_cache);
        a.mov(ARG2, x86::qword_ptr(ARG2, offsetof(ErtsCodeMFA, module)));
        a.mov(ARG3, ARG4);
        runtime_call<const void *(*)(ErtsMsAcc *, Eterm, const void *),
                     erts_msacc_set_bif_state>();
        a.mov(ARG4, RET);
        a.mov(ARG3, TMP_MEM1q);
        a.mov(ARG5, TMP_MEM2q);
        a.bind(skip_msacc);
    }
#endif
    a.mov(ARG1, c_p);
    load_x_reg_array(ARG2);
    runtime_call<Eterm (*)(Process *, Eterm *, ErtsCodePtr, ErtsBifFunc, Uint),
                 beam_jit_call_bif>();
#ifdef ERTS_MSACC_EXTENDED_STATES
    a.mov(TMP_MEM1q, RET);
    a.lea(ARG1, erts_msacc_cache);
    runtime_call<void (*)(ErtsMsAcc **), erts_msacc_update_cache>();
    a.mov(RET, TMP_MEM1q);
#endif
    emit_bif_nif_epilogue();
}
void BeamGlobalAssembler::emit_dispatch_bif(void) {
    a.mov(ARG3, x86::qword_ptr(c_p, offsetof(Process, i)));
    ERTS_CT_ASSERT(offsetof(ErtsNativeFunc, trampoline.call_bif_nif) ==
                   sizeof(ErtsCodeInfo));
    ssize_t mfa_offset = offsetof(ErtsNativeFunc, trampoline.info.mfa) -
                         offsetof(ErtsNativeFunc, trampoline.call_bif_nif);
    a.lea(ARG2, x86::qword_ptr(ARG3, mfa_offset));
    ssize_t dfunc_offset = offsetof(ErtsNativeFunc, trampoline.dfunc) -
                           offsetof(ErtsNativeFunc, trampoline.call_bif_nif);
    a.mov(ARG4, x86::qword_ptr(ARG3, dfunc_offset));
    a.jmp(labels[call_bif_shared]);
}
void BeamModuleAssembler::emit_call_bif(const ArgWord &Func) {
    int mfa_offset = -(int)sizeof(ErtsCodeMFA);
    Label entry = a.new_label();
    emit_enter_frame();
    a.bind(entry);
    {
        a.lea(ARG2, x86::qword_ptr(current_label, mfa_offset));
        a.lea(ARG3, x86::qword_ptr(entry));
        mov_arg(ARG4, Func);
        a.jmp(resolve_fragment(ga->get_call_bif_shared()));
    }
}
void BeamModuleAssembler::emit_call_bif_mfa(const ArgAtom &M,
                                            const ArgAtom &F,
                                            const ArgWord &A) {
    const Export *e;
    UWord func;
    e = erts_active_export_entry(M.get(), F.get(), A.get());
    ASSERT(e != NULL && e->bif_number != -1);
    comment("HBIF: %T:%T/%d",
            e->info.mfa.module,
            e->info.mfa.function,
            A.get());
    func = (UWord)bif_table[e->bif_number].f;
    emit_call_bif(ArgWord(func));
}
void BeamGlobalAssembler::emit_call_nif_early() {
    a.mov(ARG2, x86::qword_ptr(x86::rsp));
    a.sub(ARG2, imm(sizeof(UWord) + sizeof(ErtsCodeInfo)));
#ifdef DEBUG
    {
        Label next = a.new_label();
        a.test(ARG2, imm(sizeof(UWord) - 1));
        a.short_().je(next);
        comment("# Return address isn't word-aligned");
        a.ud2();
        a.bind(next);
    }
#endif
    emit_enter_runtime();
    a.mov(ARG1, c_p);
    runtime_call<ErtsCodePtr (*)(Process *, const ErtsCodeInfo *),
                 erts_call_nif_early>();
    emit_leave_runtime();
    a.add(x86::rsp, imm(8));
    emit_enter_frame();
    a.mov(ARG3, RET);
    a.jmp(labels[call_nif_shared]);
}
void BeamGlobalAssembler::emit_call_nif_shared(void) {
    emit_enter_runtime<Update::eReductions | Update::eStack | Update::eHeap>();
#ifdef ERTS_MSACC_EXTENDED_STATES
    {
        Label skip_msacc = a.new_label();
        a.cmp(erts_msacc_cache, 0);
        a.short_().je(skip_msacc);
        a.mov(TMP_MEM1q, ARG3);
        a.mov(ARG1, erts_msacc_cache);
        a.mov(ARG2, imm(ERTS_MSACC_STATE_NIF));
        a.mov(ARG3, imm(1));
        runtime_call<void (*)(ErtsMsAcc *, Uint, int),
                     erts_msacc_set_state_m__>();
        a.mov(ARG3, TMP_MEM1q);
        a.bind(skip_msacc);
    }
#endif
    a.mov(ARG1, c_p);
    a.mov(ARG2, ARG3);
    load_x_reg_array(ARG3);
    a.mov(ARG4, x86::qword_ptr(ARG2, 8 + BEAM_ASM_FUNC_PROLOGUE_SIZE));
    a.mov(ARG5, x86::qword_ptr(ARG2, 16 + BEAM_ASM_FUNC_PROLOGUE_SIZE));
    a.mov(ARG6, x86::qword_ptr(ARG2, 24 + BEAM_ASM_FUNC_PROLOGUE_SIZE));
    runtime_call<Eterm (*)(Process *,
                           ErtsCodePtr,
                           Eterm *,
                           BeamJitNifF *,
                           struct erl_module_nif *),
                 beam_jit_call_nif>();
    emit_bif_nif_epilogue();
}
void BeamGlobalAssembler::emit_dispatch_nif(void) {
    ERTS_CT_ASSERT(offsetof(ErtsNativeFunc, trampoline.call_bif_nif) ==
                   sizeof(ErtsCodeInfo));
    a.mov(ARG3, x86::qword_ptr(c_p, offsetof(Process, i)));
    a.jmp(labels[call_nif_shared]);
}
void BeamGlobalAssembler::emit_call_nif_yield_helper() {
    Label yield = a.new_label();
    if (erts_alcu_enable_code_atags) {
        a.mov(x86::qword_ptr(c_p, offsetof(Process, i)), ARG3);
    }
    a.dec(FCALLS);
    a.short_().jl(yield);
    a.jmp(labels[call_nif_shared]);
    a.bind(yield);
    {
        int mfa_offset = -(int)sizeof(ErtsCodeMFA);
        int arity_offset = mfa_offset + (int)offsetof(ErtsCodeMFA, arity);
        a.movzx(ARG1d, x86::byte_ptr(ARG3, arity_offset));
        a.mov(x86::byte_ptr(c_p, offsetof(Process, arity)), ARG1.r8());
        a.lea(ARG1, x86::qword_ptr(ARG3, mfa_offset));
        a.mov(x86::qword_ptr(c_p, offsetof(Process, current)), ARG1);
        a.add(ARG3, imm(BEAM_ASM_NFUNC_SIZE + sizeof(UWord[3])));
        a.jmp(labels[context_switch_simplified]);
    }
}
void BeamModuleAssembler::emit_call_nif(const ArgWord &Func,
                                        const ArgWord &NifMod,
                                        const ArgWord &DirtyFunc) {
    Label entry = a.new_label(), dispatch = a.new_label();
    a.bind(entry);
    {
        emit_enter_frame();
        a.short_().jmp(dispatch);
        a.align(AlignMode::kCode, 8);
        a.embed_uint64(Func.get());
        a.embed_uint64(NifMod.get());
        a.embed_uint64(DirtyFunc.get());
    }
    ASSERT((a.offset() - code.label_offset_from_base(current_label)) ==
           BEAM_ASM_NFUNC_SIZE + sizeof(UWord[3]));
    a.bind(dispatch);
    {
        a.lea(ARG3, x86::qword_ptr(current_label));
        pic_jmp(ga->get_call_nif_yield_helper());
    }
}
void BeamGlobalAssembler::emit_i_load_nif_shared() {
    static ErtsCodeMFA bif_mfa = {am_erlang, am_load_nif, 2};
    Label yield = a.new_label(), error = a.new_label();
    a.mov(TMP_MEM1q, ARG2);
    emit_enter_runtime<Update::eHeapAlloc>();
    a.mov(ARG1, c_p);
    load_x_reg_array(ARG3);
    runtime_call<
            enum beam_jit_nif_load_ret (*)(Process *, ErtsCodePtr, Eterm *),
            beam_jit_load_nif>();
    emit_leave_runtime<Update::eHeapAlloc>();
    a.cmp(RET, RET_NIF_yield);
    a.short_().je(yield);
    emit_leave_frame();
    a.cmp(RET, RET_NIF_success);
    a.short_().jne(error);
    a.ret();
    a.bind(error);
    {
        a.mov(ARG4, imm(&bif_mfa));
        a.jmp(labels[raise_exception]);
    }
    a.bind(yield);
    {
        a.mov(ARG3, TMP_MEM1q);
        a.jmp(labels[context_switch_simplified]);
    }
}
static ErtsCodePtr get_on_load_address(Process *c_p, Eterm module) {
    const Module *modp = erts_get_module(module, erts_active_code_ix());
    if (modp && modp->on_load) {
        const BeamCodeHeader *hdr = (modp->on_load)->code_hdr;
        if (hdr) {
            return erts_codeinfo_to_code(hdr->on_load);
        }
    }
    c_p->freason = BADARG;
    return NULL;
}
void BeamModuleAssembler::emit_i_call_on_load_function() {
    static ErtsCodeMFA mfa = {am_erlang, am_call_on_load_function, 1};
    Label next = a.new_label();
    emit_enter_runtime();
    a.mov(ARG1, c_p);
    a.mov(ARG2, getXRef(0));
    runtime_call<ErtsCodePtr (*)(Process *, Eterm), get_on_load_address>();
    emit_leave_runtime();
    a.test(RET, RET);
    a.jne(next);
    emit_raise_exception(&mfa);
    a.bind(next);
    erlang_call(RET, ARG1);
}
#ifdef NATIVE_ERLANG_STACK
void BeamModuleAssembler::emit_i_load_nif() {
    Label entry = a.new_label(), yield = a.new_label(), next = a.new_label();
    fragment_call(entry);
    a.short_().jmp(next);
    align_erlang_cp();
    a.bind(entry);
    {
        emit_enter_frame();
        a.bind(yield);
        {
            a.lea(ARG2, x86::qword_ptr(yield));
            a.jmp(resolve_fragment(ga->get_i_load_nif_shared()));
        }
    }
    a.bind(next);
}
#else
void BeamModuleAssembler::emit_i_load_nif() {
    static ErtsCodeMFA mfa = {am_erlang, am_load_nif, 2};
    Label entry = a.new_label(), next = a.new_label(), schedule = a.new_label();
    align_erlang_cp();
    a.bind(entry);
    emit_enter_runtime<Update::eHeapAlloc>();
    a.mov(ARG1, c_p);
    a.lea(ARG2, x86::qword_ptr(current_label));
    load_x_reg_array(ARG3);
    runtime_call<beam_jit_nif_load_ret (*)(Process *, ErtsCodePtr, Eterm *),
                 beam_jit_load_nif>();
    emit_leave_runtime<Update::eHeapAlloc>();
    a.cmp(RET, imm(RET_NIF_yield));
    a.je(schedule);
    a.cmp(RET, imm(RET_NIF_success));
    a.je(next);
    emit_raise_exception(current_label, &mfa);
    a.bind(schedule);
    {
        a.lea(ARG3, x86::qword_ptr(entry));
        a.jmp(resolve_fragment(ga->get_context_switch_simplified()));
    }
    a.bind(next);
}
#endif