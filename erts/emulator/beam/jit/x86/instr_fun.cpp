#include "beam_asm.hpp"
void BeamGlobalAssembler::emit_unloaded_fun() {
    Label error = a.new_label();
    emit_enter_frame();
    a.mov(TMP_MEM1q, ARG5);
    emit_enter_runtime<Update::eHeapAlloc | Update::eReductions>();
    a.mov(ARG1, c_p);
    load_x_reg_array(ARG2);
    a.shr(ARG3, imm(FUN_HEADER_ARITY_OFFS));
    a.mov(ARG5, active_code_ix);
    runtime_call<
            const Export *(*)(Process *, Eterm *, int, Eterm, ErtsCodeIndex),
            beam_jit_handle_unloaded_fun>();
    emit_leave_runtime<Update::eHeapAlloc | Update::eReductions |
                       Update::eCodeIndex>();
    a.test(RET, RET);
    a.jz(error);
    emit_leave_frame();
    a.jmp(emit_setup_dispatchable_call(RET));
    a.bind(error);
    {
        a.push(TMP_MEM1q);
        mov_imm(ARG4, nullptr);
        a.jmp(labels[raise_exception]);
    }
}
void BeamGlobalAssembler::emit_handle_call_fun_error() {
    Label bad_arity = a.new_label(), bad_fun = a.new_label();
    emit_enter_frame();
    emit_is_boxed(bad_fun, ARG4);
    x86::Gp fun_thing = emit_ptr_val(RET, ARG4);
    ERTS_CT_ASSERT(FUN_HEADER_ARITY_OFFS == 8);
    a.cmp(emit_boxed_val(fun_thing, 0, sizeof(byte)), imm(FUN_SUBTAG));
    a.short_().je(bad_arity);
    a.bind(bad_fun);
    {
        a.mov(x86::qword_ptr(c_p, offsetof(Process, freason)), imm(EXC_BADFUN));
        a.mov(x86::qword_ptr(c_p, offsetof(Process, fvalue)), ARG4);
        a.push(ARG5);
        mov_imm(ARG4, nullptr);
        a.jmp(labels[raise_exception]);
    }
    a.bind(bad_arity);
    {
        a.mov(TMP_MEM1q, ARG4);
        a.mov(TMP_MEM2q, ARG5);
        emit_enter_runtime<Update::eHeapAlloc>();
        a.mov(ARG1, c_p);
        load_x_reg_array(ARG2);
        a.shr(ARG3, imm(FUN_HEADER_ARITY_OFFS));
        runtime_call<Eterm (*)(Process *, const Eterm *, int),
                     beam_jit_build_argument_list>();
        emit_leave_runtime<Update::eHeapAlloc>();
        a.mov(ARG1, TMP_MEM1q);
        a.mov(getXRef(0), ARG1);
        a.mov(getXRef(1), RET);
        {
            const int32_t bytes_needed = (3 + S_RESERVED) * sizeof(Eterm);
            Label after_gc = a.new_label();
            a.lea(ARG3, x86::qword_ptr(HTOP, bytes_needed));
            a.cmp(ARG3, E);
            a.short_().jbe(after_gc);
            {
                mov_imm(ARG4, 2);
                aligned_call(labels[garbage_collect]);
            }
            a.bind(after_gc);
            a.mov(ARG1, getXRef(0));
            a.mov(ARG2, getXRef(1));
            a.mov(x86::qword_ptr(HTOP), imm(make_arityval(2)));
            a.mov(x86::qword_ptr(HTOP, sizeof(Eterm[1])), ARG1);
            a.mov(x86::qword_ptr(HTOP, sizeof(Eterm[2])), ARG2);
            a.lea(ARG1, x86::qword_ptr(HTOP, TAG_PRIMARY_BOXED));
            a.add(HTOP, imm(sizeof(Eterm[3])));
        }
        a.mov(x86::qword_ptr(c_p, offsetof(Process, freason)),
              imm(EXC_BADARITY));
        a.mov(x86::qword_ptr(c_p, offsetof(Process, fvalue)), ARG1);
        a.push(TMP_MEM2q);
        mov_imm(ARG4, nullptr);
        a.jmp(labels[raise_exception]);
    }
}
void BeamGlobalAssembler::emit_dispatch_save_calls_fun() {
    a.mov(ARG1, imm(&the_active_code_index));
    a.mov(ARG1d, x86::dword_ptr(ARG1));
    a.jmp(emit_setup_dispatchable_call(RET, ARG1));
}
void BeamModuleAssembler::emit_i_lambda_trampoline(const ArgLambda &Lambda,
                                                   const ArgLabel &Lbl,
                                                   const ArgWord &Arity,
                                                   const ArgWord &NumFree) {
    const ssize_t effective_arity = Arity.get() - NumFree.get();
    const ssize_t num_free = NumFree.get();
    const auto &lambda = lambdas[Lambda.get()];
    a.bind(lambda.trampoline);
    emit_ptr_val(ARG4, ARG4);
    ASSERT(num_free > 0);
    emit_copy_words(emit_boxed_val(ARG4, offsetof(ErlFunThing, env)),
                    getXRef(effective_arity),
                    num_free,
                    RET);
    a.jmp(resolve_beam_label(Lbl));
}
void BeamModuleAssembler::emit_i_make_fun3(const ArgLambda &Lambda,
                                           const ArgRegister &Dst,
                                           const ArgWord &Arity,
                                           const ArgWord &NumFree,
                                           const Span<const ArgVal> &env) {
    ASSERT((NumFree.get()) == env.size() &&
           (NumFree.get() + Arity.get()) < MAX_ARG);
    mov_arg(RET, Lambda);
    comment("Create fun thing");
    preserve_cache([&]() {
        a.mov(x86::qword_ptr(HTOP, offsetof(ErlFunThing, thing_word)),
              imm(MAKE_FUN_HEADER(Arity.get(), NumFree.get(), 0)));
        a.mov(x86::qword_ptr(HTOP, offsetof(ErlFunThing, entry.fun)), RET);
    });
    comment("Move fun environment");
    for (Uint i = 0; i < env.size(); i++) {
        const ArgVal &next = (i + 1) < env.size() ? env[i + 1] : ArgNil();
        switch (ArgVal::memory_relation(env[i], next)) {
        case ArgVal::Relation::consecutive: {
            x86::Mem src_ptr = getArgRef(env[i].as<ArgRegister>(), 16);
            x86::Mem dst_ptr = x86::xmmword_ptr(HTOP,
                                                offsetof(ErlFunThing, env) +
                                                        i * sizeof(Eterm));
            comment("(moving two items)");
            preserve_cache([&]() {
                vmovups(x86::xmm0, src_ptr);
                vmovups(dst_ptr, x86::xmm0);
            });
            i++;
            break;
        }
        case ArgVal::Relation::reverse_consecutive: {
            if (!hasCpuFeature(CpuFeatures::X86::kAVX)) {
                goto fallback;
            }
            x86::Mem src_ptr = getArgRef(env[i + 1].as<ArgRegister>(), 16);
            x86::Mem dst_ptr = x86::xmmword_ptr(HTOP,
                                                offsetof(ErlFunThing, env) +
                                                        i * sizeof(Eterm));
            comment("(moving and swapping two items)");
            preserve_cache([&]() {
                a.vpermilpd(x86::xmm0, src_ptr, 1);
                a.vmovups(dst_ptr, x86::xmm0);
            });
            i++;
            break;
        }
        case ArgVal::Relation::none:
        fallback:
            mov_arg(x86::qword_ptr(HTOP,
                                   offsetof(ErlFunThing, env) +
                                           i * sizeof(Eterm)),
                    env[i]);
            break;
        }
    }
    comment("Create boxed ptr");
    preserve_cache(
            [&]() {
                a.lea(RET, x86::qword_ptr(HTOP, TAG_PRIMARY_BOXED));
                a.add(HTOP, imm((ERL_FUN_SIZE + env.size()) * sizeof(Eterm)));
            },
            RET,
            HTOP);
    mov_arg(Dst, RET);
}
void BeamGlobalAssembler::emit_apply_fun_shared() {
    Label finished = a.new_label();
    emit_enter_frame();
    mov_imm(ARG3, 0);
    a.mov(ARG4, getXRef(0));
    a.mov(ARG5, getXRef(1));
    {
        Label unpack_next = a.new_label(), malformed_list = a.new_label(),
              raise_error = a.new_label();
        auto x_register = getXRef(0);
        ASSERT(x_register.shift() == 0);
        x_register.set_index(ARG3);
        x_register.set_shift(3);
        a.mov(ARG1, ARG5);
        a.bind(unpack_next);
        {
            a.cmp(ARG1d, imm(NIL));
            a.short_().je(finished);
            a.test(ARG1.r8(), imm(_TAG_PRIMARY_MASK - TAG_PRIMARY_LIST));
            a.short_().jne(malformed_list);
            emit_ptr_val(ARG1, ARG1);
            a.mov(RET, getCARRef(ARG1));
            a.mov(ARG1, getCDRRef(ARG1));
            a.mov(x_register, RET);
            a.inc(ARG3);
            a.cmp(ARG3, imm(MAX_REG - 1));
            a.jb(unpack_next);
        }
        a.mov(RET, imm(SYSTEM_LIMIT));
        a.jmp(raise_error);
        a.bind(malformed_list);
        a.mov(RET, imm(BADARG));
        a.bind(raise_error);
        {
            static const ErtsCodeMFA apply_mfa = {am_erlang, am_apply, 2};
            a.mov(getXRef(0), ARG4);
            a.mov(getXRef(1), ARG5);
            a.mov(x86::qword_ptr(c_p, offsetof(Process, freason)), RET);
            mov_imm(ARG4, &apply_mfa);
            emit_leave_frame();
            a.jmp(labels[raise_exception]);
        }
    }
    a.bind(finished);
    a.shl(ARG3, imm(FUN_HEADER_ARITY_OFFS));
    a.or_(ARG3, imm(FUN_SUBTAG));
    emit_leave_frame();
    a.ret();
}
void BeamModuleAssembler::emit_i_apply_fun() {
    safe_fragment_call(ga->get_apply_fun_shared());
    x86::Gp target = emit_call_fun();
    ASSERT(target != ARG6);
    erlang_call(target, ARG6);
}
void BeamModuleAssembler::emit_i_apply_fun_last(const ArgWord &Deallocate) {
    emit_deallocate(Deallocate);
    emit_i_apply_fun_only();
}
void BeamModuleAssembler::emit_i_apply_fun_only() {
    safe_fragment_call(ga->get_apply_fun_shared());
    x86::Gp target = emit_call_fun();
    emit_leave_frame();
    a.jmp(target);
}
x86::Gp BeamModuleAssembler::emit_call_fun(bool skip_box_test,
                                           bool skip_header_test) {
    const bool can_fail = !(skip_box_test && skip_header_test);
    Label next = a.new_label();
    x86::Gp fun_thing = emit_ptr_val(RET, ARG4);
    if (can_fail) {
        a.mov(ARG1, ga->get_handle_call_fun_error());
    }
    a.lea(ARG5, x86::qword_ptr(next));
    if (skip_box_test) {
        comment("skipped box test since source is always boxed");
    } else {
        a.test(ARG4d, imm(_TAG_PRIMARY_MASK - TAG_PRIMARY_BOXED));
        a.short_().jne(next);
    }
    if (skip_header_test) {
        comment("skipped fun/arity test since source is always a fun of the "
                "right arity when boxed");
    } else {
        ERTS_CT_ASSERT(FUN_HEADER_ARITY_OFFS == 8);
        ERTS_CT_ASSERT(FUN_HEADER_ENV_SIZE_OFFS == 16);
        a.cmp(emit_boxed_val(fun_thing, 0, sizeof(Uint16)), ARG3.r16());
        a.short_().jne(next);
    }
    a.mov(RET, emit_boxed_val(fun_thing, offsetof(ErlFunThing, entry)));
    a.mov(ARG1, emit_setup_dispatchable_call(RET));
    a.bind(next);
    return ARG1;
}
void BeamModuleAssembler::emit_i_call_fun2(const ArgVal &Tag,
                                           const ArgWord &Arity,
                                           const ArgRegister &Func) {
    mov_arg(ARG4, Func);
    if (Tag.isImmed()) {
        mov_imm(ARG3, MAKE_FUN_HEADER(Arity.get(), 0, 0) & 0xFFFF);
        ASSERT(Tag.as<ArgImmed>().get() != am_safe || beam->types.fallback ||
               exact_type<BeamTypeId::Fun>(Func));
        auto target =
                emit_call_fun(always_one_of<BeamTypeId::AlwaysBoxed>(Func),
                              Tag.as<ArgImmed>().get() == am_safe);
        erlang_call(target, ARG6);
    } else {
        const auto &trampoline = lambdas[Tag.as<ArgLambda>().get()].trampoline;
        erlang_call(trampoline, RET);
    }
}
void BeamModuleAssembler::emit_i_call_fun2_last(const ArgVal &Tag,
                                                const ArgWord &Arity,
                                                const ArgRegister &Func,
                                                const ArgWord &Deallocate) {
    mov_arg(ARG4, Func);
    if (Tag.isImmed()) {
        mov_imm(ARG3, MAKE_FUN_HEADER(Arity.get(), 0, 0) & 0xFFFF);
        ASSERT(Tag.as<ArgImmed>().get() != am_safe || beam->types.fallback ||
               exact_type<BeamTypeId::Fun>(Func));
        auto target =
                emit_call_fun(always_one_of<BeamTypeId::AlwaysBoxed>(Func),
                              Tag.as<ArgImmed>().get() == am_safe);
        emit_deallocate(Deallocate);
        emit_leave_frame();
        a.jmp(target);
    } else {
        const auto &trampoline = lambdas[Tag.as<ArgLambda>().get()].trampoline;
        emit_deallocate(Deallocate);
        emit_leave_frame();
        a.jmp(trampoline);
    }
}
void BeamModuleAssembler::emit_i_call_fun(const ArgWord &Arity) {
    const ArgXRegister Func(Arity.get());
    const ArgAtom Tag(am_unsafe);
    emit_i_call_fun2(Tag, Arity, Func);
}
void BeamModuleAssembler::emit_i_call_fun_last(const ArgWord &Arity,
                                               const ArgWord &Deallocate) {
    const ArgXRegister Func(Arity.get());
    const ArgAtom Tag(am_unsafe);
    emit_i_call_fun2_last(Tag, Arity, Func, Deallocate);
}
void BeamModuleAssembler::emit_i_lambda_error(const ArgWord &Dummy) {
    comment("lambda error");
    a.ud2();
}