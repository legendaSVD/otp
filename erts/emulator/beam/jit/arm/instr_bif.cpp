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
void BeamModuleAssembler::ubif_comment(const ArgWord &Bif) {
    if (logger.file()) {
        ErtsCodeMFA *mfa = ubif2mfa((void *)Bif.get());
        if (mfa) {
            comment("UBIF: %T/%d", mfa->function, mfa->arity);
        }
    }
}
void BeamGlobalAssembler::emit_i_bif_guard_shared() {
    ERTS_CT_ASSERT(ERTS_HIGHEST_CALLEE_SAVE_XREG >= 2);
    emit_enter_runtime_frame();
    emit_enter_runtime<Update::eReductions>();
    a.mov(ARG1, c_p);
    lea(ARG2, getXRef(0));
    mov_imm(ARG3, 0);
    dynamic_runtime_call<3>(ARG4);
    emit_leave_runtime<Update::eReductions>();
    emit_leave_runtime_frame();
    a.ret(a64::x30);
}
void BeamGlobalAssembler::emit_i_bif_body_shared() {
    Label error = a.new_label();
    ERTS_CT_ASSERT(ERTS_HIGHEST_CALLEE_SAVE_XREG >= 2);
    emit_enter_runtime_frame();
    emit_enter_runtime<Update::eReductions>();
    a.mov(ARG1, c_p);
    lea(ARG2, getXRef(0));
    a.str(ARG4, TMP_MEM1q);
    mov_imm(ARG3, 0);
    dynamic_runtime_call<3>(ARG4);
    emit_branch_if_not_value(ARG1, error);
    emit_leave_runtime<Update::eReductions>();
    emit_leave_runtime_frame();
    a.ret(a64::x30);
    a.bind(error);
    {
        a.ldr(ARG1, TMP_MEM1q);
        runtime_call<ErtsCodeMFA *(*)(void *), ubif2mfa>();
        emit_leave_runtime<Update::eReductions | Update::eXRegs>(3);
        emit_leave_runtime_frame();
        a.mov(ARG4, ARG1);
        a.b(labels[raise_exception]);
    }
}
void BeamModuleAssembler::emit_i_bif1(const ArgSource &Src1,
                                      const ArgLabel &Fail,
                                      const ArgWord &Bif,
                                      const ArgRegister &Dst) {
    auto src1 = load_source(Src1);
    a.str(src1.reg, getXRef(0));
    ubif_comment(Bif);
    emit_i_bif(Fail, Bif, Dst);
}
void BeamModuleAssembler::emit_i_bif2(const ArgSource &Src1,
                                      const ArgSource &Src2,
                                      const ArgLabel &Fail,
                                      const ArgWord &Bif,
                                      const ArgRegister &Dst) {
    auto [src1, src2] = load_sources(Src1, TMP1, Src2, TMP2);
    a.stp(src1.reg, src2.reg, getXRef(0));
    ubif_comment(Bif);
    emit_i_bif(Fail, Bif, Dst);
}
void BeamModuleAssembler::emit_i_bif3(const ArgSource &Src1,
                                      const ArgSource &Src2,
                                      const ArgSource &Src3,
                                      const ArgLabel &Fail,
                                      const ArgWord &Bif,
                                      const ArgRegister &Dst) {
    auto [src1, src2] = load_sources(Src1, TMP1, Src2, TMP2);
    auto src3 = load_source(Src3, TMP3);
    a.stp(src1.reg, src2.reg, getXRef(0));
    a.str(src3.reg, getXRef(2));
    ubif_comment(Bif);
    emit_i_bif(Fail, Bif, Dst);
}
void BeamModuleAssembler::emit_i_bif(const ArgLabel &Fail,
                                     const ArgWord &Bif,
                                     const ArgRegister &Dst) {
    mov_arg(ARG4, Bif);
    if (Fail.get() != 0) {
        fragment_call(ga->get_i_bif_guard_shared());
        emit_branch_if_not_value(ARG1, resolve_beam_label(Fail, dispUnknown));
    } else {
        fragment_call(ga->get_i_bif_body_shared());
    }
    mov_arg(Dst, ARG1);
}
void BeamModuleAssembler::emit_nofail_bif1(const ArgSource &Src1,
                                           const ArgWord &Bif,
                                           const ArgRegister &Dst) {
    auto src1 = load_source(Src1);
    a.str(src1.reg, getXRef(0));
    ubif_comment(Bif);
    mov_arg(ARG4, Bif);
    fragment_call(ga->get_i_bif_guard_shared());
    mov_arg(Dst, ARG1);
}
void BeamModuleAssembler::emit_nofail_bif2(const ArgSource &Src1,
                                           const ArgSource &Src2,
                                           const ArgWord &Bif,
                                           const ArgRegister &Dst) {
    auto [src1, src2] = load_sources(Src1, TMP1, Src2, TMP2);
    a.stp(src1.reg, src2.reg, getXRef(0));
    ubif_comment(Bif);
    mov_arg(ARG4, Bif);
    fragment_call(ga->get_i_bif_guard_shared());
    mov_arg(Dst, ARG1);
}
void BeamModuleAssembler::emit_i_length_setup(const ArgLabel &Fail,
                                              const ArgWord &Live,
                                              const ArgSource &Src) {
    ERTS_CT_ASSERT(ERTS_X_REGS_ALLOCATED - MAX_REG >= 3);
    auto trap_reg1 = ArgXRegister(Live.get() + 0);
    auto trap_reg2 = ArgXRegister(Live.get() + 1);
    auto trap_reg3 = ArgXRegister(Live.get() + 2);
    auto src = load_source(Src, TMP1);
    auto dst1 = init_destination(trap_reg1, src.reg);
    auto dst2 = init_destination(trap_reg2, TMP2);
    mov_imm(dst2.reg, make_small(0));
    mov_var(dst1, src);
    if (Fail.get() != 0) {
        flush_vars(dst1, dst2);
    } else {
        auto dst3 = init_destination(trap_reg3, src.reg);
        mov_var(dst3, src);
        flush_vars(dst1, dst2, dst3);
    }
}
void BeamGlobalAssembler::emit_i_length_common(Label fail, int state_size) {
    Label trap_or_error = a.new_label();
    ASSERT(state_size >= 2 && state_size <= ERTS_X_REGS_ALLOCATED - MAX_REG);
    a.stp(ARG2, ARG3, TMP_MEM1q);
    emit_enter_runtime_frame();
    emit_enter_runtime<Update::eReductions | Update::eXRegs>();
    a.mov(ARG1, c_p);
    lea(TMP1, getXRef(0));
    a.add(ARG2, TMP1, ARG2, a64::lsl(3));
    runtime_call<Eterm (*)(Process *, Eterm *), erts_trapping_length_1>();
    emit_branch_if_not_value(ARG1, trap_or_error);
    emit_leave_runtime<Update::eReductions | Update::eXRegs>();
    emit_leave_runtime_frame();
    a.ret(a64::x30);
    a.bind(trap_or_error);
    {
        a.ldp(ARG2, ARG3, TMP_MEM1q);
        a.ldr(TMP1, a64::Mem(c_p, offsetof(Process, freason)));
        a.cmp(TMP1, imm(TRAP));
        a.b_ne(fail);
        emit_leave_runtime<Update::eReductions | Update::eXRegs>();
        emit_leave_runtime_frame();
        a.add(ARG2, ARG2, imm(state_size));
        a.str(ZERO, a64::Mem(c_p, offsetof(Process, current)));
        a.strb(ARG2.w(), a64::Mem(c_p, offsetof(Process, arity)));
        a.b(labels[context_switch_simplified]);
    }
}
void BeamGlobalAssembler::emit_i_length_body_shared() {
    Label error = a.new_label();
    emit_i_length_common(error, 3);
    a.bind(error);
    {
        static const ErtsCodeMFA bif_mfa = {am_erlang, am_length, 1};
        lea(TMP1, getXRef(0));
        a.add(ARG2, TMP1, ARG2, a64::lsl(3));
        a.ldr(TMP1, a64::Mem(ARG2, sizeof(Eterm[2])));
        emit_leave_runtime<Update::eReductions | Update::eXRegs>();
        emit_leave_runtime_frame();
        a.mov(XREG0, TMP1);
        mov_imm(ARG4, &bif_mfa);
        emit_raise_exception();
    }
}
void BeamGlobalAssembler::emit_i_length_guard_shared() {
    Label error = a.new_label();
    emit_i_length_common(error, 2);
    a.bind(error);
    {
        emit_leave_runtime<Update::eReductions | Update::eXRegs>();
        emit_leave_runtime_frame();
        a.ret(a64::x30);
    }
}
void BeamModuleAssembler::emit_i_length(const ArgLabel &Fail,
                                        const ArgWord &Live,
                                        const ArgRegister &Dst) {
    Label entry = a.new_label();
    a.bind(entry);
    mov_arg(ARG2, Live);
    a.adr(ARG3, entry);
    if (Fail.get() != 0) {
        fragment_call(ga->get_i_length_guard_shared());
        emit_branch_if_not_value(ARG1, resolve_beam_label(Fail, dispUnknown));
    } else {
        fragment_call(ga->get_i_length_body_shared());
    }
    mov_arg(Dst, ARG1);
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
    a64::Mem entry_mem = TMP_MEM1q, export_mem = TMP_MEM2q,
             mbuf_mem = TMP_MEM3q;
    Label trace = a.new_label(), yield = a.new_label();
    a.ldr(TMP1, a64::Mem(c_p, offsetof(Process, mbuf)));
    a.stp(ARG3, ARG4, TMP_MEM1q);
    a.str(TMP1, mbuf_mem);
    a.ldr(TMP1.w(), a64::Mem(ARG4, offsetof(Export, is_bif_traced)));
    a.cmp(TMP1, imm(0));
    a.ccmp(active_code_ix,
           imm(ERTS_SAVE_CALLS_CODE_IX),
           imm(NZCV::kZF),
           imm(arm::CondCode::kEQ));
    a.b_eq(trace);
    a.subs(FCALLS, FCALLS, imm(1));
    a.b_le(yield);
    {
        Label check_bif_return = a.new_label(),
              gc_after_bif_call = a.new_label();
        emit_enter_runtime_frame();
        emit_enter_runtime<Update::eReductions | Update::eStack |
                           Update::eHeap | Update::eXRegs>(MAX_BIF_ARITY);
#ifdef ERTS_MSACC_EXTENDED_STATES
        {
            Label skip_msacc = a.new_label();
            a.ldr(TMP1, erts_msacc_cache);
            a.cbz(TMP1, skip_msacc);
            a.mov(XREG0, ARG3);
            a.ldr(ARG1, erts_msacc_cache);
            a.ldr(ARG2, a64::Mem(ARG4, offsetof(Export, info.mfa.module)));
            a.mov(ARG3, ARG8);
            runtime_call<const void *(*)(ErtsMsAcc *, Eterm, const void *),
                         erts_msacc_set_bif_state>();
            a.mov(ARG8, ARG1);
            a.mov(ARG3, XREG0);
            a.bind(skip_msacc);
        }
#endif
        {
            a.mov(ARG1, c_p);
            load_x_reg_array(ARG2);
#if defined(DEBUG) || defined(ERTS_ENABLE_LOCK_CHECK)
            a.mov(ARG4, ARG8);
            runtime_call<
                    Eterm (*)(Process *, Eterm *, ErtsCodePtr, ErtsBifFunc),
                    debug_call_light_bif>();
#else
            dynamic_runtime_call<3>(ARG8);
#endif
        }
#ifdef ERTS_MSACC_EXTENDED_STATES
        {
            Label skip_msacc = a.new_label();
            a.mov(XREG0, ARG1);
            a.ldr(TMP1, erts_msacc_cache);
            a.cbz(TMP1, skip_msacc);
            lea(ARG1, erts_msacc_cache);
            runtime_call<void (*)(ErtsMsAcc **), erts_msacc_update_cache>();
            a.ldr(ARG1, erts_msacc_cache);
            a.cbz(ARG1, skip_msacc);
            mov_imm(ARG2, ERTS_MSACC_STATE_EMULATOR);
            mov_imm(ARG3, 1);
            runtime_call<void (*)(ErtsMsAcc *, Uint, int),
                         erts_msacc_set_state_m__>();
            a.bind(skip_msacc);
            a.mov(ARG1, XREG0);
        }
#endif
        emit_leave_runtime<Update::eReductions | Update::eCodeIndex |
                           Update::eHeap | Update::eStack | Update::eXRegs>(
                MAX_BIF_ARITY);
        emit_leave_runtime_frame();
        {
            a.ldr(TMP1.w(), a64::Mem(c_p, offsetof(Process, flags)));
            a.tst(TMP1, imm(F_FORCE_GC | F_DISABLE_GC));
            a.ldr(TMP1, a64::Mem(c_p, offsetof(Process, bin_vheap_sz)));
            a.ldr(TMP2, a64::Mem(c_p, offsetof(Process, off_heap.overhead)));
            a.ccmp(TMP2, TMP1, imm(NZCV::kCF), imm(arm::CondCode::kEQ));
            a.sub(TMP1, E, HTOP);
            a.asr(TMP1, TMP1, imm(3));
            a.ldr(TMP2, a64::Mem(c_p, offsetof(Process, mbuf_sz)));
            a.ccmp(TMP1, TMP2, imm(NZCV::kVF), imm(arm::CondCode::kLS));
            a.b_lt(gc_after_bif_call);
        }
        a.bind(check_bif_return);
        {
            Label error = a.new_label(), trap = a.new_label();
            emit_branch_if_not_value(ARG1, trap);
            a.mov(XREG0, ARG1);
            a.ret(a64::x30);
            a.bind(trap);
            {
                a.ldr(TMP1, a64::Mem(c_p, offsetof(Process, freason)));
                emit_branch_if_ne(TMP1, TRAP, error);
                emit_enter_erlang_frame();
                a.ldr(ARG3, a64::Mem(c_p, offsetof(Process, i)));
                a.b(labels[context_switch_simplified]);
            }
            a.bind(error);
            {
                a.ldp(ARG2, ARG4, entry_mem);
                add(ARG4, ARG4, offsetof(Export, info.mfa));
                a.b(labels[raise_exception_shared]);
            }
        }
        a.bind(gc_after_bif_call);
        {
            emit_enter_runtime_frame();
            emit_enter_runtime<Update::eReductions | Update::eStack |
                               Update::eHeap | Update::eXRegs>(MAX_BIF_ARITY);
            a.mov(ARG3, ARG1);
            a.mov(ARG1, c_p);
            a.ldr(ARG2, mbuf_mem);
            load_x_reg_array(ARG4);
            a.ldr(ARG5, export_mem);
            a.ldrb(ARG5.w(), a64::Mem(ARG5, offsetof(Export, info.mfa.arity)));
            runtime_call<Eterm (*)(Process *,
                                   ErlHeapFragment *,
                                   Eterm,
                                   Eterm *,
                                   Uint),
                         erts_gc_after_bif_call_lhf>();
            emit_leave_runtime<Update::eReductions | Update::eStack |
                               Update::eHeap | Update::eXRegs>(MAX_BIF_ARITY);
            emit_leave_runtime_frame();
            a.b(check_bif_return);
        }
    }
    a.bind(trace);
    {
        branch(emit_setup_dispatchable_call(ARG4));
    }
    a.bind(yield);
    {
        a.ldrb(ARG2.w(), a64::Mem(ARG4, offsetof(Export, info.mfa.arity)));
        lea(ARG4, a64::Mem(ARG4, offsetof(Export, info.mfa)));
        a.strb(ARG2.w(), a64::Mem(c_p, offsetof(Process, arity)));
        a.str(ARG4, a64::Mem(c_p, offsetof(Process, current)));
        a.b(labels[context_switch_simplified]);
    }
}
void BeamModuleAssembler::emit_call_light_bif(const ArgWord &Bif,
                                              const ArgExport &Exp) {
    Label entry = a.new_label();
    BeamFile_ImportEntry *e = &beam->imports.entries[Exp.get()];
    a.bind(entry);
    mov_arg(ARG4, Exp);
    mov_arg(ARG8, Bif);
    a.adr(ARG3, entry);
    if (logger.file()) {
        comment("BIF: %T:%T/%d", e->module, e->function, e->arity);
    }
    fragment_call(ga->get_call_light_bif_shared());
}
void BeamModuleAssembler::emit_send() {
    Label entry = a.new_label();
    a.bind(entry);
    a.ldr(ARG4, embed_constant(BIF_TRAP_EXPORT(BIF_send_2), disp32K));
    a.ldr(ARG8, embed_constant(send_2, disp32K));
    a.adr(ARG3, entry);
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
        a.ldr(TMP1, erts_msacc_cache);
        a.cbz(TMP1, skip_msacc);
        a.mov(XREG0, ARG1);
        a.ldr(ARG1, erts_msacc_cache);
        mov_imm(ARG2, ERTS_MSACC_STATE_EMULATOR);
        mov_imm(ARG3, 1);
        runtime_call<void (*)(ErtsMsAcc *, Uint, int),
                     erts_msacc_set_state_m__>();
        a.mov(ARG1, XREG0);
        a.bind(skip_msacc);
    }
#endif
    emit_leave_runtime<Update::eStack | Update::eHeap | Update::eXRegs |
                       Update::eReductions | Update::eCodeIndex>();
    emit_branch_if_not_value(ARG1, check_trap);
    comment("Do return and dispatch to it");
    a.mov(XREG0, ARG1);
    emit_leave_erlang_frame();
    if (erts_alcu_enable_code_atags) {
        a.str(a64::x30, a64::Mem(c_p, offsetof(Process, i)));
    }
    a.ret(a64::x30);
    a.bind(check_trap);
    a.ldr(TMP1, a64::Mem(c_p, offsetof(Process, freason)));
    a.cmp(TMP1, imm(TRAP));
    a.b_ne(error);
    {
        comment("yield");
        comment("test trap to hibernate");
        a.ldr(TMP1.w(), a64::Mem(c_p, offsetof(Process, flags)));
        a.tbz(TMP1, imm(Support::ctz(F_HIBERNATE_SCHED)), trap);
        comment("do hibernate trap");
        a.and_(TMP1, TMP1, imm(~F_HIBERNATE_SCHED));
        a.str(TMP1.w(), a64::Mem(c_p, offsetof(Process, flags)));
        a.b(labels[do_schedule]);
    }
    a.bind(trap);
    {
        comment("do normal trap");
        a.ldr(ARG3, a64::Mem(c_p, offsetof(Process, i)));
        a.b(labels[context_switch_simplified]);
    }
    a.bind(error);
    {
        a.mov(ARG2, E);
        emit_enter_runtime();
        a.mov(ARG1, c_p);
        runtime_call<ErtsCodePtr (*)(const Process *, const Eterm *),
                     erts_printable_return_address>();
        emit_leave_runtime();
        a.mov(ARG2, ARG1);
        a.ldr(ARG4, a64::Mem(c_p, offsetof(Process, current)));
        a.b(labels[raise_exception_shared]);
    }
}
void BeamGlobalAssembler::emit_call_bif_shared(void) {
    emit_enter_runtime_frame();
    a.str(ARG2, a64::Mem(c_p, offsetof(Process, current)));
    a.ldr(ARG5.w(), a64::Mem(ARG2, offsetof(ErtsCodeMFA, arity)));
    a.strb(ARG5.w(), a64::Mem(c_p, offsetof(Process, arity)));
    a.str(ARG3, a64::Mem(c_p, offsetof(Process, i)));
    emit_enter_runtime<Update::eStack | Update::eHeap | Update::eXRegs |
                       Update::eReductions>();
#ifdef ERTS_MSACC_EXTENDED_STATES
    {
        Label skip_msacc = a.new_label();
        a.ldr(TMP1, erts_msacc_cache);
        a.cbz(TMP1, skip_msacc);
        a.mov(XREG0, ARG3);
        a.mov(XREG1, ARG5);
        a.ldr(ARG1, erts_msacc_cache);
        a.ldr(ARG2, a64::Mem(ARG2, offsetof(ErtsCodeMFA, module)));
        a.mov(ARG3, ARG4);
        runtime_call<const void *(*)(ErtsMsAcc *, Eterm, const void *),
                     erts_msacc_set_bif_state>();
        a.mov(ARG4, ARG1);
        a.mov(ARG3, XREG0);
        a.mov(ARG5, XREG1);
        a.bind(skip_msacc);
    }
#endif
    a.mov(ARG1, c_p);
    load_x_reg_array(ARG2);
    runtime_call<Eterm (*)(Process *, Eterm *, ErtsCodePtr, ErtsBifFunc, Uint),
                 beam_jit_call_bif>();
#ifdef ERTS_MSACC_EXTENDED_STATES
    a.mov(XREG0, ARG1);
    lea(ARG1, erts_msacc_cache);
    runtime_call<void (*)(ErtsMsAcc **), erts_msacc_update_cache>();
    a.mov(ARG1, XREG0);
#endif
    emit_leave_runtime_frame();
    emit_bif_nif_epilogue();
}
void BeamGlobalAssembler::emit_dispatch_bif(void) {
    a.ldr(ARG3, a64::Mem(c_p, offsetof(Process, i)));
    ERTS_CT_ASSERT(offsetof(ErtsNativeFunc, trampoline.call_bif_nif) ==
                   sizeof(ErtsCodeInfo));
    ssize_t mfa_offset = offsetof(ErtsNativeFunc, trampoline.call_bif_nif) -
                         offsetof(ErtsNativeFunc, trampoline.info.mfa);
    a.sub(ARG2, ARG3, imm(mfa_offset));
    ssize_t dfunc_offset = offsetof(ErtsNativeFunc, trampoline.dfunc) -
                           offsetof(ErtsNativeFunc, trampoline.call_bif_nif);
    a.ldr(ARG4, a64::Mem(ARG3, dfunc_offset));
    a.b(labels[call_bif_shared]);
}
void BeamModuleAssembler::emit_call_bif(const ArgWord &Func) {
    (void)Func;
    emit_nyi("emit_call_bif");
}
void BeamModuleAssembler::emit_call_bif_mfa(const ArgAtom &M,
                                            const ArgAtom &F,
                                            const ArgWord &A) {
    const Export *e;
    UWord func;
    e = erts_active_export_entry(M.get(), F.get(), A.get());
    ASSERT(e != NULL && e->bif_number != -1);
    func = (UWord)bif_table[e->bif_number].f;
    a.adr(ARG3, current_label);
    a.sub(ARG2, ARG3, imm(sizeof(ErtsCodeMFA)));
    comment("HBIF: %T:%T/%d",
            e->info.mfa.module,
            e->info.mfa.function,
            A.get());
    a.mov(ARG4, imm(func));
    a.b(resolve_fragment(ga->get_call_bif_shared(), disp128MB));
}
void BeamGlobalAssembler::emit_call_nif_early() {
    a.mov(ARG2, a64::x30);
    a.sub(ARG2, ARG2, imm(BEAM_ASM_FUNC_PROLOGUE_SIZE + sizeof(ErtsCodeInfo)));
    emit_enter_runtime();
    a.mov(ARG1, c_p);
    runtime_call<ErtsCodePtr (*)(Process *, const ErtsCodeInfo *),
                 erts_call_nif_early>();
    emit_leave_runtime();
    a.mov(ARG3, ARG1);
    a.b(labels[call_nif_shared]);
}
void BeamGlobalAssembler::emit_call_nif_shared(void) {
    emit_enter_runtime<Update::eStack | Update::eHeap | Update::eXRegs |
                       Update::eReductions>();
#ifdef ERTS_MSACC_EXTENDED_STATES
    {
        Label skip_msacc = a.new_label();
        a.ldr(TMP1, erts_msacc_cache);
        a.cbz(TMP1, skip_msacc);
        a.mov(XREG0, ARG3);
        a.ldr(ARG1, erts_msacc_cache);
        mov_imm(ARG2, ERTS_MSACC_STATE_NIF);
        mov_imm(ARG3, 1);
        runtime_call<void (*)(ErtsMsAcc *, Uint, int),
                     erts_msacc_set_state_m__>();
        a.mov(ARG3, XREG0);
        a.bind(skip_msacc);
    }
#endif
    a.mov(ARG1, c_p);
    a.mov(ARG2, ARG3);
    load_x_reg_array(ARG3);
    ERTS_CT_ASSERT((4 + BEAM_ASM_FUNC_PROLOGUE_SIZE) % sizeof(UWord) == 0);
    a.ldr(ARG4, a64::Mem(ARG2, 4 + BEAM_ASM_FUNC_PROLOGUE_SIZE));
    a.ldr(ARG5, a64::Mem(ARG2, 12 + BEAM_ASM_FUNC_PROLOGUE_SIZE));
    a.ldr(ARG6, a64::Mem(ARG2, 16 + BEAM_ASM_FUNC_PROLOGUE_SIZE));
    runtime_call<Eterm (*)(Process *,
                           ErtsCodePtr,
                           Eterm *,
                           BeamJitNifF *,
                           struct erl_module_nif *),
                 beam_jit_call_nif>();
    emit_bif_nif_epilogue();
}
void BeamGlobalAssembler::emit_dispatch_nif(void) {
    a.ldr(ARG3, a64::Mem(c_p, offsetof(Process, i)));
    a.b(labels[call_nif_shared]);
}
void BeamGlobalAssembler::emit_call_nif_yield_helper() {
    Label yield = a.new_label();
    if (erts_alcu_enable_code_atags) {
        a.str(ARG3, a64::Mem(c_p, offsetof(Process, i)));
    }
    a.subs(FCALLS, FCALLS, imm(1));
    a.b_le(yield);
    a.b(labels[call_nif_shared]);
    a.bind(yield);
    {
        int mfa_offset = sizeof(ErtsCodeMFA);
        int arity_offset = offsetof(ErtsCodeMFA, arity) - mfa_offset;
        a.ldur(TMP1.w(), a64::Mem(ARG3, arity_offset));
        a.strb(TMP1.w(), a64::Mem(c_p, offsetof(Process, arity)));
        a.sub(TMP1, ARG3, imm(mfa_offset));
        a.str(TMP1, a64::Mem(c_p, offsetof(Process, current)));
        a.add(ARG3, ARG3, imm(BEAM_ASM_NFUNC_SIZE + sizeof(UWord[3])));
        a.b(labels[context_switch_simplified]);
    }
}
void BeamModuleAssembler::emit_call_nif(const ArgWord &Func,
                                        const ArgWord &NifMod,
                                        const ArgWord &DirtyFunc) {
    Label entry = a.new_label(), dispatch = a.new_label();
    a.bind(entry);
    {
        a.b(dispatch);
        ASSERT(a.offset() % sizeof(UWord) == 0);
        a.embed_uint64(Func.get());
        a.embed_uint64(NifMod.get());
        a.embed_uint64(DirtyFunc.get());
    }
    ASSERT((a.offset() - code.label_offset_from_base(current_label)) ==
           BEAM_ASM_NFUNC_SIZE + sizeof(UWord[3]));
    a.bind(dispatch);
    {
        a.adr(ARG3, current_label);
        pic_jmp(ga->get_call_nif_yield_helper());
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
    a.mov(ARG2, XREG0);
    emit_enter_runtime(1);
    a.mov(ARG1, c_p);
    runtime_call<ErtsCodePtr (*)(Process *, Eterm), get_on_load_address>();
    emit_leave_runtime(1);
    a.cbnz(ARG1, next);
    emit_raise_exception(&mfa);
    a.bind(next);
    erlang_call(ARG1);
}
void BeamModuleAssembler::emit_i_load_nif() {
    static ErtsCodeMFA mfa = {am_erlang, am_load_nif, 2};
    Label entry = a.new_label(), next = a.new_label(), schedule = a.new_label();
    a.bind(entry);
    emit_enter_runtime<Update::eHeapAlloc | Update::eXRegs>(2);
    a.mov(ARG1, c_p);
    a.adr(ARG2, current_label);
    load_x_reg_array(ARG3);
    runtime_call<beam_jit_nif_load_ret (*)(Process *, ErtsCodePtr, Eterm *),
                 beam_jit_load_nif>();
    emit_leave_runtime<Update::eHeapAlloc | Update::eXRegs>(2);
    a.cmp(ARG1, imm(RET_NIF_yield));
    a.b_eq(schedule);
    a.cmp(ARG1, imm(RET_NIF_success));
    a.b_eq(next);
    emit_raise_exception(current_label, &mfa);
    a.bind(schedule);
    {
        a.adr(ARG3, entry);
        a.b(resolve_fragment(ga->get_context_switch_simplified(), disp128MB));
    }
    a.bind(next);
}