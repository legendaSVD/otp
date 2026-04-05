#include <algorithm>
#include <numeric>
#include "beam_asm.hpp"
extern "C"
{
#include "erl_bif_table.h"
#include "big.h"
#include "beam_catches.h"
#include "beam_common.h"
#include "code_ix.h"
#include "erl_binary.h"
#include "erl_map.h"
}
using namespace asmjit;
void BeamModuleAssembler::emit_error(int reason) {
    a.mov(x86::qword_ptr(c_p, offsetof(Process, freason)), imm(reason));
    emit_raise_exception();
}
void BeamModuleAssembler::emit_gc_test_preserve(const ArgWord &Need,
                                                const ArgWord &Live,
                                                const ArgSource &Preserve,
                                                x86::Gp preserve_reg) {
    const int32_t bytes_needed = (Need.get() + S_RESERVED) * sizeof(Eterm);
    Label after_gc_check = a.new_label();
    ASSERT(preserve_reg != ARG3);
#ifdef DEBUG
    comment("(debug: fill dead X registers with garbage)");
    const x86::Gp garbage_reg = ARG3;
    mov_imm(garbage_reg, ERTS_HOLE_MARKER);
    if (!(Preserve.isXRegister() &&
          Preserve.as<ArgXRegister>().get() >= Live.get())) {
        mov_arg(ArgXRegister(Live.get()), garbage_reg);
        mov_arg(ArgXRegister(Live.get() + 1), garbage_reg);
    } else {
        mov_arg(ArgXRegister(Live.get() + 1), garbage_reg);
        mov_arg(ArgXRegister(Live.get() + 2), garbage_reg);
    }
#endif
    a.lea(ARG3, x86::qword_ptr(HTOP, bytes_needed));
    a.cmp(ARG3, E);
    a.short_().jbe(after_gc_check);
    if (!(Preserve.isXRegister() &&
          Preserve.as<ArgXRegister>().get() >= Live.get())) {
        mov_imm(ARG4, Live.get());
        fragment_call(ga->get_garbage_collect());
        mov_arg(preserve_reg, Preserve);
    } else {
        a.mov(getXRef(Live.get()), preserve_reg);
        mov_imm(ARG4, Live.get() + 1);
        fragment_call(ga->get_garbage_collect());
        a.mov(preserve_reg, getXRef(Live.get()));
    }
    a.bind(after_gc_check);
}
void BeamModuleAssembler::emit_gc_test(const ArgWord &Ns,
                                       const ArgWord &Nh,
                                       const ArgWord &Live) {
    const int32_t bytes_needed =
            (Ns.get() + Nh.get() + S_RESERVED) * sizeof(Eterm);
    Label after_gc_check = a.new_label();
#ifdef DEBUG
    comment("(debug: fill dead X registers with garbage)");
    mov_imm(ARG4, ERTS_HOLE_MARKER);
    mov_arg(ArgXRegister(Live.get()), ARG4);
    mov_arg(ArgXRegister(Live.get() + 1), ARG4);
#endif
    a.lea(ARG3, x86::qword_ptr(HTOP, bytes_needed));
    a.cmp(ARG3, E);
    a.short_().jbe(after_gc_check);
    mov_imm(ARG4, Live.get());
    fragment_call(ga->get_garbage_collect());
    a.bind(after_gc_check);
}
void BeamModuleAssembler::emit_validate(const ArgWord &Arity) {
#ifdef DEBUG
    Label next = a.new_label(), crash = a.new_label();
    a.test(HTOP, imm(sizeof(Eterm) - 1));
    a.jne(crash);
    a.test(E, imm(sizeof(Eterm) - 1));
    a.jne(crash);
    a.lea(ARG1, x86::qword_ptr(E, -(int32_t)(S_REDZONE * sizeof(Eterm))));
    a.cmp(HTOP, ARG1);
    a.ja(crash);
    a.jmp(next);
    a.bind(crash);
    a.hlt();
    a.bind(next);
#    ifdef JIT_HARD_DEBUG
    emit_enter_runtime();
    for (unsigned i = 0; i < Arity.get(); i++) {
        a.mov(ARG1, getXRef(i));
        runtime_call<void (*)(Eterm), beam_jit_validate_term>();
    }
    emit_leave_runtime();
#    endif
#endif
}
void BeamModuleAssembler::emit_i_validate(const ArgWord &Arity) {
    emit_validate(Arity);
}
void BeamModuleAssembler::emit_allocate_heap(const ArgWord &NeedStack,
                                             const ArgWord &NeedHeap,
                                             const ArgWord &Live) {
    ASSERT(NeedStack.get() <= MAX_REG);
    ArgWord needed = NeedStack;
#if !defined(NATIVE_ERLANG_STACK)
    needed = needed + CP_SIZE;
#endif
    emit_gc_test(needed, NeedHeap, Live);
    if (needed.get() > 0) {
        a.sub(E, imm(needed.get() * sizeof(Eterm)));
    }
#if !defined(NATIVE_ERLANG_STACK)
    a.mov(getCPRef(), imm(NIL));
#endif
}
void BeamModuleAssembler::emit_allocate(const ArgWord &NeedStack,
                                        const ArgWord &Live) {
    emit_allocate_heap(NeedStack, ArgWord(0), Live);
}
void BeamModuleAssembler::emit_deallocate(const ArgWord &Deallocate) {
    ASSERT(Deallocate.get() <= 1023);
    if (ERTS_LIKELY(erts_frame_layout == ERTS_FRAME_LAYOUT_RA)) {
        ArgWord dealloc = Deallocate;
#if !defined(NATIVE_ERLANG_STACK)
        dealloc = dealloc + CP_SIZE;
#endif
        if (dealloc.get() > 0) {
            a.add(E, imm(dealloc.get() * sizeof(Eterm)));
        }
    } else {
        ASSERT(erts_frame_layout == ERTS_FRAME_LAYOUT_FP_RA);
    }
}
void BeamModuleAssembler::emit_test_heap(const ArgWord &Nh,
                                         const ArgWord &Live) {
    emit_gc_test(ArgWord(0), Nh, Live);
}
void BeamModuleAssembler::emit_normal_exit() {
    emit_enter_runtime<Update::eReductions | Update::eHeapAlloc>();
    emit_proc_lc_unrequire();
    a.mov(x86::qword_ptr(c_p, offsetof(Process, freason)), imm(EXC_NORMAL));
    a.mov(x86::byte_ptr(c_p, offsetof(Process, arity)), imm(0));
    a.mov(ARG1, c_p);
    mov_imm(ARG2, am_normal);
    runtime_call<void (*)(Process *, Eterm), erts_do_exit_process>();
    emit_proc_lc_require();
    emit_leave_runtime<Update::eReductions | Update::eHeapAlloc>();
    a.jmp(resolve_fragment(ga->get_do_schedule()));
}
void BeamModuleAssembler::emit_continue_exit() {
    emit_enter_runtime<Update::eReductions | Update::eHeapAlloc>();
    emit_proc_lc_unrequire();
    a.mov(ARG1, c_p);
    runtime_call<void (*)(Process *), erts_continue_exit_process>();
    emit_proc_lc_require();
    emit_leave_runtime<Update::eReductions | Update::eHeapAlloc>();
    a.jmp(resolve_fragment(ga->get_do_schedule()));
}
void BeamModuleAssembler::emit_get_list(const x86::Gp src,
                                        const ArgRegister &Hd,
                                        const ArgRegister &Tl) {
    x86::Gp boxed_ptr = emit_ptr_val(src, src);
    switch (ArgVal::memory_relation(Hd, Tl)) {
    case ArgVal::Relation::consecutive: {
        comment("(moving head and tail together)");
        x86::Mem dst_ptr = getArgRef(Hd, 16);
        x86::Mem src_ptr = getCARRef(boxed_ptr, 16);
        preserve_cache(
                [&]() {
                    vmovups(x86::xmm0, src_ptr);
                    vmovups(dst_ptr, x86::xmm0);
                },
                getArgRef(Hd),
                getArgRef(Tl));
        break;
    }
    case ArgVal::Relation::reverse_consecutive: {
        if (!hasCpuFeature(CpuFeatures::X86::kAVX)) {
            goto fallback;
        }
        comment("(moving and swapping head and tail together)");
        x86::Mem dst_ptr = getArgRef(Tl, 16);
        x86::Mem src_ptr = getCARRef(boxed_ptr, 16);
        preserve_cache(
                [&]() {
                    a.vpermilpd(x86::xmm0, src_ptr, 1);
                    a.vmovups(dst_ptr, x86::xmm0);
                },
                getArgRef(Hd),
                getArgRef(Tl));
        break;
    }
    case ArgVal::Relation::none:
    fallback:
        preserve_cache(
                [&]() {
                    a.mov(ARG2, getCARRef(boxed_ptr));
                    a.mov(ARG3, getCDRRef(boxed_ptr));
                },
                ARG2,
                ARG3);
        mov_arg(Hd, ARG2);
        mov_arg(Tl, ARG3);
        break;
    }
}
void BeamModuleAssembler::emit_get_list(const ArgRegister &Src,
                                        const ArgRegister &Hd,
                                        const ArgRegister &Tl) {
    mov_arg(ARG1, Src);
    emit_get_list(ARG1, Hd, Tl);
}
void BeamModuleAssembler::emit_get_hd(const ArgRegister &Src,
                                      const ArgRegister &Hd) {
    mov_arg(ARG1, Src);
    x86::Gp boxed_ptr = emit_ptr_val(ARG1, ARG1);
    mov_preserve_cache(ARG2, getCARRef(boxed_ptr));
    mov_arg(Hd, ARG2);
}
void BeamModuleAssembler::emit_get_tl(const ArgRegister &Src,
                                      const ArgRegister &Tl) {
    mov_arg(ARG1, Src);
    x86::Gp boxed_ptr = emit_ptr_val(ARG1, ARG1);
    mov_preserve_cache(ARG2, getCDRRef(boxed_ptr));
    mov_arg(Tl, ARG2);
}
void BeamModuleAssembler::emit_is_nonempty_list_get_list(
        const ArgLabel &Fail,
        const ArgRegister &Src,
        const ArgRegister &Hd,
        const ArgRegister &Tl) {
    mov_arg(RET, Src);
    emit_is_cons(resolve_beam_label(Fail), RET);
    emit_get_list(RET, Hd, Tl);
}
void BeamModuleAssembler::emit_is_nonempty_list_get_hd(const ArgLabel &Fail,
                                                       const ArgRegister &Src,
                                                       const ArgRegister &Hd) {
    mov_arg(RET, Src);
    emit_is_cons(resolve_beam_label(Fail), RET);
    x86::Gp ptr = emit_ptr_val(RET, RET);
    mov_preserve_cache(ARG2, getCARRef(ptr));
    mov_arg(Hd, ARG2);
}
void BeamModuleAssembler::emit_is_nonempty_list_get_tl(const ArgLabel &Fail,
                                                       const ArgRegister &Src,
                                                       const ArgRegister &Tl) {
    mov_arg(RET, Src);
    emit_is_cons(resolve_beam_label(Fail), RET);
    x86::Gp ptr = emit_ptr_val(RET, RET);
    mov_preserve_cache(ARG2, getCDRRef(ptr));
    mov_arg(Tl, ARG2);
}
void BeamModuleAssembler::emit_i_get(const ArgSource &Src,
                                     const ArgRegister &Dst) {
    mov_arg(ARG2, Src);
    emit_enter_runtime();
    a.mov(ARG1, c_p);
    runtime_call<Eterm (*)(Process *, Eterm), erts_pd_hash_get>();
    emit_leave_runtime();
    mov_arg(Dst, RET);
}
void BeamModuleAssembler::emit_i_get_hash(const ArgConstant &Src,
                                          const ArgWord &Hash,
                                          const ArgRegister &Dst) {
    mov_arg(ARG2, Hash);
    mov_arg(ARG3, Src);
    emit_enter_runtime();
    a.mov(ARG1, c_p);
    runtime_call<Eterm (*)(Process *, erts_ihash_t, Eterm),
                 erts_pd_hash_get_with_hx>();
    emit_leave_runtime();
    mov_arg(Dst, RET);
}
void BeamModuleAssembler::emit_load_tuple_ptr(const ArgSource &Term) {
    mov_arg(ARG2, Term);
    (void)emit_ptr_val(ARG2, ARG2);
}
#ifdef DEBUG
void BeamModuleAssembler::emit_tuple_assertion(const ArgSource &Src,
                                               x86::Gp tuple_reg) {
    Label ok = a.new_label(), fatal = a.new_label();
    ASSERT(tuple_reg != RET);
    mov_arg(RET, Src);
    emit_is_boxed(fatal, RET, dShort);
    (void)emit_ptr_val(RET, RET);
    a.cmp(RET, tuple_reg);
    a.short_().je(ok);
    a.bind(fatal);
    {
        comment("tuple assertion failure");
        a.ud2();
    }
    a.bind(ok);
}
#endif
void BeamModuleAssembler::emit_i_get_tuple_element(const ArgSource &Src,
                                                   const ArgWord &Element,
                                                   const ArgRegister &Dst) {
    x86::Gp tmp_reg = alloc_temp_reg();
#ifdef DEBUG
    emit_tuple_assertion(Src, ARG2);
#endif
    preserve_cache(
            [&]() {
                a.mov(tmp_reg, emit_boxed_val(ARG2, Element.get()));
            },
            tmp_reg);
    mov_arg(Dst, tmp_reg);
}
void BeamModuleAssembler::emit_get_tuple_element_swap(
        const ArgSource &Src,
        const ArgWord &Element,
        const ArgRegister &Dst,
        const ArgRegister &OtherDst) {
#ifdef DEBUG
    emit_tuple_assertion(Src, ARG2);
#endif
    preserve_cache(
            [&]() {
                mov_arg(ARG1, OtherDst);
                a.mov(ARG3, emit_boxed_val(ARG2, Element.get()));
            },
            ARG1,
            ARG3);
    mov_arg(Dst, ARG1);
    mov_arg(OtherDst, ARG3);
}
void BeamModuleAssembler::emit_get_two_tuple_elements(const ArgSource &Src,
                                                      const ArgWord &Element,
                                                      const ArgRegister &Dst1,
                                                      const ArgRegister &Dst2) {
#ifdef DEBUG
    emit_tuple_assertion(Src, ARG2);
#endif
    x86::Mem element_ptr =
            emit_boxed_val(ARG2, Element.get(), 2 * sizeof(Eterm));
    switch (ArgVal::memory_relation(Dst1, Dst2)) {
    case ArgVal::Relation::consecutive: {
        x86::Mem dst_ptr = getArgRef(Dst1, 16);
        preserve_cache(
                [&]() {
                    vmovups(x86::xmm0, element_ptr);
                    vmovups(dst_ptr, x86::xmm0);
                },
                getArgRef(Dst1),
                getArgRef(Dst2));
        break;
    }
    case ArgVal::Relation::reverse_consecutive: {
        if (!hasCpuFeature(CpuFeatures::X86::kAVX)) {
            goto fallback;
        } else {
            x86::Mem dst_ptr = getArgRef(Dst2, 16);
            preserve_cache(
                    [&]() {
                        a.vpermilpd(x86::xmm0,
                                    element_ptr,
                                    1);
                        a.vmovups(dst_ptr, x86::xmm0);
                    },
                    getArgRef(Dst1),
                    getArgRef(Dst2));
            break;
        }
    }
    case ArgVal::Relation::none:
    fallback:
        a.mov(ARG1, emit_boxed_val(ARG2, Element.get()));
        a.mov(ARG3, emit_boxed_val(ARG2, Element.get() + sizeof(Eterm)));
        mov_arg(Dst1, ARG1);
        mov_arg(Dst2, ARG3);
        break;
    }
}
void BeamModuleAssembler::emit_init_yregs(const ArgWord &Size,
                                          const Span<const ArgVal> &args) {
    unsigned count = Size.get();
    ASSERT(count == args.size());
    if (count == 1) {
        mov_arg(args.first(), NIL);
        return;
    }
    unsigned i = 0;
    int y_ptr = -1;
    mov_imm(x86::rax, NIL);
    while (i < count) {
        unsigned first_y = args[i].as<ArgYRegister>().get();
        unsigned slots = 1;
        while (i + slots < count) {
            const ArgYRegister &current_y = args[i + slots];
            if (first_y + slots != current_y.get()) {
                break;
            }
            slots++;
        }
        if (slots == 1) {
            a.mov(getYRef(first_y), x86::rax);
        } else {
            if (first_y == 0) {
#ifdef NATIVE_ERLANG_STACK
                a.mov(x86::rdi, E);
#else
                a.lea(x86::rdi, getYRef(0));
#endif
                y_ptr = 0;
            } else if (y_ptr < 0) {
                y_ptr = first_y;
                a.lea(x86::rdi, getYRef(y_ptr));
            } else {
                unsigned offset = (first_y - y_ptr) * sizeof(Eterm);
                a.add(x86::rdi, imm(offset));
                y_ptr = first_y;
            }
            if (slots <= 4) {
                for (unsigned j = 0; j < slots; j++) {
                    a.stosq();
                }
            } else {
                mov_imm(x86::rcx, slots);
                a.rep().stosq();
            }
            y_ptr += slots;
        }
        i += slots;
    }
}
void BeamModuleAssembler::emit_i_trim(const ArgWord &Words) {
    trim_preserve_cache(Words);
}
void BeamModuleAssembler::emit_i_move(const ArgSource &Src,
                                      const ArgRegister &Dst) {
    x86::Gp spill = alloc_temp_reg();
    mov_arg(Dst, Src, spill);
}
void BeamModuleAssembler::emit_move_two_words(const ArgSource &Src1,
                                              const ArgRegister &Dst1,
                                              const ArgSource &Src2,
                                              const ArgRegister &Dst2) {
    x86::Mem src_ptr = getArgRef(Src1, 16);
    ASSERT(ArgVal::memory_relation(Src1, Src2) ==
           ArgVal::Relation::consecutive);
    switch (ArgVal::memory_relation(Dst1, Dst2)) {
    case ArgVal::Relation::consecutive: {
        x86::Mem dst_ptr = getArgRef(Dst1, 16);
        preserve_cache(
                [&]() {
                    vmovups(x86::xmm0, src_ptr);
                    vmovups(dst_ptr, x86::xmm0);
                },
                getArgRef(Dst1),
                getArgRef(Dst2));
        break;
    }
    case ArgVal::Relation::reverse_consecutive: {
        x86::Mem dst_ptr = getArgRef(Dst2, 16);
        comment("(moving and swapping)");
        if (hasCpuFeature(CpuFeatures::X86::kAVX)) {
            preserve_cache(
                    [&]() {
                        a.vpermilpd(x86::xmm0, src_ptr, 1);
                        a.vmovups(dst_ptr, x86::xmm0);
                    },
                    getArgRef(Dst1),
                    getArgRef(Dst2));
        } else {
            mov_arg(ARG1, Src1);
            mov_arg(ARG2, Src2);
            mov_arg(Dst1, ARG1);
            mov_arg(Dst2, ARG2);
        }
        break;
    }
    case ArgVal::Relation::none:
        ASSERT(0);
        break;
    }
}
void BeamModuleAssembler::emit_swap(const ArgRegister &R1,
                                    const ArgRegister &R2) {
    if (!hasCpuFeature(CpuFeatures::X86::kAVX)) {
        goto fallback;
    }
    switch (ArgVal::memory_relation(R1, R2)) {
    case ArgVal::Relation::consecutive: {
        x86::Mem ptr = getArgRef(R1, 16);
        comment("(swapping using AVX)");
        preserve_cache(
                [&]() {
                    a.vpermilpd(x86::xmm0, ptr, 1);
                    a.vmovups(ptr, x86::xmm0);
                },
                getArgRef(R1),
                getArgRef(R2));
        break;
    }
    case ArgVal::Relation::reverse_consecutive: {
        x86::Mem ptr = getArgRef(R2, 16);
        comment("(swapping using AVX)");
        preserve_cache(
                [&]() {
                    a.vpermilpd(x86::xmm0, ptr, 1);
                    a.vmovups(ptr, x86::xmm0);
                },
                getArgRef(R1),
                getArgRef(R2));
        break;
    }
    case ArgVal::Relation::none:
    fallback:
        mov_arg(ARG1, R1);
        mov_arg(ARG2, R2);
        mov_arg(R2, ARG1);
        mov_arg(R1, ARG2);
        break;
    }
}
void BeamModuleAssembler::emit_node(const ArgRegister &Dst) {
    x86::Gp reg = alloc_temp_reg();
    preserve_cache(
            [&]() {
                a.mov(reg, imm(&erts_this_node));
                a.mov(reg, x86::qword_ptr(reg));
                a.mov(reg, x86::qword_ptr(reg, offsetof(ErlNode, sysname)));
            },
            reg);
    mov_arg(Dst, reg);
}
void BeamModuleAssembler::emit_put_cons(const ArgSource &Hd,
                                        const ArgSource &Tl) {
    switch (ArgVal::memory_relation(Hd, Tl)) {
    case ArgVal::Relation::consecutive: {
        x86::Mem src_ptr = getArgRef(Hd, 16);
        x86::Mem dst_ptr = x86::xmmword_ptr(HTOP, 0);
        comment("(put head and tail together)");
        preserve_cache([&]() {
            vmovups(x86::xmm0, src_ptr);
            vmovups(dst_ptr, x86::xmm0);
        });
        break;
    }
    case ArgVal::Relation::reverse_consecutive: {
        if (!hasCpuFeature(CpuFeatures::X86::kAVX)) {
            goto fallback;
        }
        x86::Mem src_ptr = getArgRef(Tl, 16);
        x86::Mem dst_ptr = x86::xmmword_ptr(HTOP, 0);
        comment("(putting and swapping head and tail together)");
        preserve_cache([&]() {
            a.vpermilpd(x86::xmm0, src_ptr, 1);
            a.vmovups(dst_ptr, x86::xmm0);
        });
        break;
    }
    case ArgVal::Relation::none:
    fallback:
        mov_arg(x86::qword_ptr(HTOP, 0), Hd);
        mov_arg(x86::qword_ptr(HTOP, 1 * sizeof(Eterm)), Tl);
        break;
    }
    preserve_cache(
            [&]() {
                a.lea(ARG2, x86::qword_ptr(HTOP, TAG_PRIMARY_LIST));
            },
            ARG2);
}
void BeamModuleAssembler::emit_append_cons(const ArgWord &Index,
                                           const ArgSource &Hd) {
    size_t offset = Index.get() * sizeof(Eterm[2]);
    mov_arg(x86::qword_ptr(HTOP, offset), Hd);
    preserve_cache(
            [&]() {
                a.mov(x86::qword_ptr(HTOP, offset + sizeof(Eterm)), ARG2);
                a.lea(ARG2, x86::qword_ptr(HTOP, offset + TAG_PRIMARY_LIST));
            },
            ARG2);
}
void BeamModuleAssembler::emit_store_cons(const ArgWord &Len,
                                          const ArgRegister &Dst) {
    preserve_cache(
            [&]() {
                a.add(HTOP, imm(Len.get() * sizeof(Eterm[2])));
            },
            HTOP);
    mov_arg(Dst, ARG2);
}
void BeamModuleAssembler::emit_put_tuple2(const ArgRegister &Dst,
                                          const ArgWord &Arity,
                                          const Span<const ArgVal> &args) {
    size_t size = args.size();
    ArgVal value = ArgWord(0);
    ASSERT(arityval(Arity.get()) == size);
    comment("Move arity word");
    mov_arg(x86::qword_ptr(HTOP, 0), Arity);
    comment("Move tuple data");
    for (unsigned i = 0; i < size; i++) {
        x86::Mem dst_ptr = x86::qword_ptr(HTOP, (i + 1) * sizeof(Eterm));
        if (i + 1 == size) {
            mov_arg(dst_ptr, args[i]);
        } else {
            switch (ArgVal::memory_relation(args[i], args[i + 1])) {
            case ArgVal::Relation::consecutive: {
                x86::Mem src_ptr = getArgRef(args[i], 16);
                comment("(moving two elements at once)");
                dst_ptr.set_size(16);
                preserve_cache([&]() {
                    vmovups(x86::xmm0, src_ptr);
                    vmovups(dst_ptr, x86::xmm0);
                });
                i++;
                break;
            }
            case ArgVal::Relation::reverse_consecutive: {
                if (!hasCpuFeature(CpuFeatures::X86::kAVX)) {
                    mov_arg(dst_ptr, args[i]);
                } else {
                    x86::Mem src_ptr = getArgRef(args[i + 1], 16);
                    comment("(moving and swapping two elements at once)");
                    dst_ptr.set_size(16);
                    preserve_cache([&]() {
                        a.vpermilpd(x86::xmm0, src_ptr, 1);
                        a.vmovups(dst_ptr, x86::xmm0);
                    });
                    i++;
                }
                break;
            }
            case ArgVal::Relation::none: {
                unsigned j;
                if (value == args[i]) {
                    mov_preserve_cache(dst_ptr, RET);
                    break;
                }
                for (j = i + 1; j < size && args[i] == args[j]; j++) {
                    ;
                }
                if (j - i < 2) {
                    mov_arg(dst_ptr, args[i]);
                } else {
                    value = args[i];
                    mov_arg(RET, value);
                    while (i < j) {
                        dst_ptr = x86::qword_ptr(HTOP, (i + 1) * sizeof(Eterm));
                        preserve_cache([&]() {
                            a.mov(dst_ptr, RET);
                        });
                        i++;
                    }
                    i--;
                }
                break;
            }
            }
        }
    }
    comment("Create boxed ptr");
    x86::Gp tmp_reg = alloc_temp_reg();
    preserve_cache(
            [&]() {
                a.lea(tmp_reg, x86::qword_ptr(HTOP, TAG_PRIMARY_BOXED));
                a.add(HTOP, imm((size + 1) * sizeof(Eterm)));
            },
            HTOP,
            tmp_reg);
    mov_arg(Dst, tmp_reg);
}
void BeamModuleAssembler::emit_self(const ArgRegister &Dst) {
    x86::Gp reg = alloc_temp_reg();
    preserve_cache(
            [&]() {
                a.mov(reg, x86::qword_ptr(c_p, offsetof(Process, common.id)));
            },
            reg);
    mov_arg(Dst, reg);
}
void BeamModuleAssembler::emit_update_record(
        const ArgAtom &Hint,
        const ArgWord &TupleSize,
        const ArgSource &Src,
        const ArgRegister &Dst,
        const ArgWord &UpdateCount,
        const Span<const ArgVal> &updates) {
    size_t copy_index = 0, size_on_heap = TupleSize.get() + 1;
    Label next = a.new_label();
    x86::Gp ptr_val;
    ASSERT(UpdateCount.get() == updates.size());
    ASSERT((UpdateCount.get() % 2) == 0);
    ASSERT(size_on_heap > 2);
    mov_arg(RET, Src);
    if (Hint.get() == am_reuse && updates.size() == 2) {
        const auto next_index = updates[0].as<ArgWord>().get();
        const auto &next_value = updates[1].as<ArgSource>();
        a.mov(ARG1, RET);
        ptr_val = emit_ptr_val(ARG1, ARG1);
        cmp_arg(emit_boxed_val(ptr_val, next_index * sizeof(Eterm)),
                next_value,
                ARG2);
        a.je(next);
    }
    ptr_val = emit_ptr_val(RET, RET);
    for (size_t i = 0; i < updates.size(); i += 2) {
        const auto next_index = updates[i].as<ArgWord>().get();
        const auto &next_value = updates[i + 1].as<ArgSource>();
        ASSERT(next_index > 0 && next_index >= copy_index);
        emit_copy_words(emit_boxed_val(ptr_val, copy_index * sizeof(Eterm)),
                        x86::qword_ptr(HTOP, copy_index * sizeof(Eterm)),
                        next_index - copy_index,
                        ARG1);
        mov_arg(x86::qword_ptr(HTOP, next_index * sizeof(Eterm)),
                next_value,
                ARG1);
        copy_index = next_index + 1;
    }
    emit_copy_words(emit_boxed_val(ptr_val, copy_index * sizeof(Eterm)),
                    x86::qword_ptr(HTOP, copy_index * sizeof(Eterm)),
                    size_on_heap - copy_index,
                    ARG1);
    a.lea(RET, x86::qword_ptr(HTOP, TAG_PRIMARY_BOXED));
    a.add(HTOP, imm(size_on_heap * sizeof(Eterm)));
    a.bind(next);
    mov_arg(Dst, RET);
}
void BeamModuleAssembler::emit_update_record_in_place(
        const ArgWord &TupleSize,
        const ArgSource &Src,
        const ArgRegister &Dst,
        const ArgWord &UpdateCount,
        const Span<const ArgVal> &updates) {
    bool all_safe = true;
    ArgSource maybe_immediate = ArgNil();
    const size_t size_on_heap = TupleSize.get() + 1;
    ASSERT(UpdateCount.get() == updates.size());
    ASSERT((UpdateCount.get() % 2) == 0);
    ASSERT(size_on_heap > 2);
    for (size_t i = 0; i < updates.size(); i += 2) {
        const auto &value = updates[i + 1].as<ArgSource>();
        if (!(always_immediate(value) || value.isLiteral())) {
            all_safe = false;
            if (maybe_immediate.isNil() &&
                always_one_of<BeamTypeId::MaybeImmediate>(value)) {
                maybe_immediate = value;
            } else {
                maybe_immediate = ArgNil();
                break;
            }
        }
    }
    x86::Gp tagged_ptr = RET;
    mov_arg(tagged_ptr, Src);
#if defined(DEBUG) && defined(TAG_LITERAL_PTR)
    {
        Label not_literal = a.new_label();
        a.test(tagged_ptr, imm(TAG_LITERAL_PTR));
        a.short_().je(not_literal);
        a.ud2();
        a.bind(not_literal);
    }
#endif
    if (all_safe) {
        comment("skipped copy fallback because all new values are safe");
    } else {
        Label update = a.new_label();
        if (!maybe_immediate.isNil()) {
            mov_arg(ARG4, maybe_immediate);
            preserve_cache([&]() {
                emit_is_boxed(update, ARG4, dShort);
            });
        }
        preserve_cache(
                [&]() {
                    Label copy = a.new_label();
                    a.mov(ARG1, x86::Mem(c_p, offsetof(Process, high_water)));
                    a.cmp(tagged_ptr, HTOP);
                    a.short_().jae(copy);
                    a.cmp(tagged_ptr, ARG1);
                    a.short_().jae(update);
                    a.bind(copy);
                    emit_copy_words(emit_boxed_val(tagged_ptr, 0),
                                    x86::qword_ptr(HTOP, 0),
                                    size_on_heap,
                                    ARG1);
                    a.lea(RET, x86::qword_ptr(HTOP, TAG_PRIMARY_BOXED));
                    a.add(HTOP, imm(size_on_heap * sizeof(Eterm)));
                    a.bind(update);
                },
                ARG1);
    }
    for (size_t i = 0; i < updates.size(); i += 2) {
        const auto next_index = updates[i].as<ArgWord>().get();
        const auto &next_value = updates[i + 1].as<ArgSource>();
        ASSERT(next_index > 0);
        mov_arg(emit_boxed_val(RET, next_index * sizeof(Eterm)),
                next_value,
                ARG1);
    }
    mov_arg(Dst, RET);
#ifdef DEBUG
    if (!all_safe && maybe_immediate.isNil()) {
        Label bad_pointer = a.new_label(), pointer_ok = a.new_label();
        comment("sanity-checking tuple pointer");
        a.mov(ARG1, x86::Mem(c_p, offsetof(Process, heap)));
        a.cmp(RET, HTOP);
        a.short_().jae(bad_pointer);
        a.cmp(RET, ARG1);
        a.short_().jae(pointer_ok);
        a.bind(bad_pointer);
        {
            emit_enter_runtime();
            a.mov(ARG1, c_p);
            a.mov(ARG2, RET);
            runtime_call<void (*)(Process *, Eterm),
                         beam_jit_invalid_heap_ptr>();
            emit_leave_runtime();
        }
        a.bind(pointer_ok);
    }
#endif
}
void BeamModuleAssembler::emit_set_tuple_element(const ArgSource &Element,
                                                 const ArgRegister &Tuple,
                                                 const ArgWord &Offset) {
    mov_arg(ARG1, Tuple);
    x86::Gp boxed_ptr = emit_ptr_val(ARG1, ARG1);
    mov_arg(emit_boxed_val(boxed_ptr, Offset.get()), Element, ARG2);
}
void BeamModuleAssembler::emit_is_nonempty_list(const ArgLabel &Fail,
                                                const ArgRegister &Src) {
    x86::Mem list_ptr = getArgRef(Src, 1);
    preserve_cache([&]() {
        a.test(list_ptr, imm(_TAG_PRIMARY_MASK - TAG_PRIMARY_LIST));
        a.jne(resolve_beam_label(Fail));
    });
}
void BeamModuleAssembler::emit_jump(const ArgLabel &Fail) {
    a.jmp(resolve_beam_label(Fail));
}
void BeamModuleAssembler::emit_is_atom(const ArgLabel &Fail,
                                       const ArgSource &Src) {
    mov_arg(RET, Src);
    if (always_one_of<BeamTypeId::Atom, BeamTypeId::AlwaysBoxed>(Src)) {
        comment("simplified atom test since all other types are boxed");
        emit_is_not_boxed(resolve_beam_label(Fail), RET);
    } else {
        preserve_cache(
                [&]() {
                    ERTS_CT_ASSERT(_TAG_IMMED2_MASK < 256);
                    a.and_(RETb, imm(_TAG_IMMED2_MASK));
                    a.cmp(RETb, imm(_TAG_IMMED2_ATOM));
                    a.jne(resolve_beam_label(Fail));
                },
                RET);
    }
}
void BeamModuleAssembler::emit_is_boolean(const ArgLabel &Fail,
                                          const ArgSource &Src) {
    ERTS_CT_ASSERT(am_false == make_atom(0));
    ERTS_CT_ASSERT(am_true == make_atom(1));
    mov_arg(ARG1, Src);
    a.and_(ARG1, imm(~(am_true & ~_TAG_IMMED2_MASK)));
    a.cmp(ARG1, imm(am_false));
    a.jne(resolve_beam_label(Fail));
}
void BeamModuleAssembler::emit_is_bitstring(const ArgLabel &Fail,
                                            const ArgSource &Src) {
    mov_arg(ARG1, Src);
    emit_is_boxed(resolve_beam_label(Fail), Src, ARG1);
    if (masked_types<BeamTypeId::MaybeBoxed>(Src) == BeamTypeId::Bitstring) {
        comment("skipped header test since we know it's a bitstring when "
                "boxed");
    } else {
        x86::Gp boxed_ptr = emit_ptr_val(ARG1, ARG1);
        preserve_cache(
                [&]() {
                    a.mov(RETd, emit_boxed_val(boxed_ptr, 0, sizeof(Uint32)));
                    a.and_(RETb, imm(_BITSTRING_TAG_MASK));
                    a.cmp(RETb, imm(_TAG_HEADER_HEAP_BITS));
                    a.jne(resolve_beam_label(Fail));
                },
                RET);
    }
}
void BeamModuleAssembler::emit_is_binary(const ArgLabel &Fail,
                                         const ArgSource &Src) {
    Label not_sub_bits = a.new_label();
    mov_arg(ARG1, Src);
    emit_is_boxed(resolve_beam_label(Fail), Src, ARG1);
    x86::Gp boxed_ptr = emit_ptr_val(ARG1, ARG1);
    preserve_cache(
            [&]() {
                ERTS_CT_ASSERT(offsetof(ErlHeapBits, size) == sizeof(Eterm));
                a.mov(RET,
                      emit_boxed_val(boxed_ptr, offsetof(ErlHeapBits, size)));
                a.mov(ARG2d, emit_boxed_val(boxed_ptr, 0, sizeof(Uint32)));
                a.cmp(ARG2d, imm(HEADER_SUB_BITS));
                a.short_().jne(not_sub_bits);
                {
                    a.mov(RET,
                          emit_boxed_val(boxed_ptr, offsetof(ErlSubBits, end)));
                    a.sub(RET,
                          emit_boxed_val(boxed_ptr,
                                         offsetof(ErlSubBits, start)));
                }
            },
            RET,
            ARG2);
    a.bind(not_sub_bits);
    preserve_cache(
            [&]() {
                ERTS_CT_ASSERT((7u << (32 - 3)) > _BITSTRING_TAG_MASK);
                a.shl(RETd, imm(32 - 3));
            },
            RET);
    if (masked_types<BeamTypeId::MaybeBoxed>(Src) == BeamTypeId::Bitstring) {
        comment("skipped header test since we know it's a bitstring when "
                "boxed");
    } else {
        preserve_cache(
                [&]() {
                    a.and_(ARG2d, imm(_BITSTRING_TAG_MASK));
                    a.or_(ARG2d, RETd);
                    a.cmp(ARG2d, imm(_TAG_HEADER_HEAP_BITS));
                },
                ARG2);
    }
    preserve_cache([&]() {
        a.jne(resolve_beam_label(Fail));
    });
}
void BeamModuleAssembler::emit_is_float(const ArgLabel &Fail,
                                        const ArgSource &Src) {
    mov_arg(ARG1, Src);
    emit_is_boxed(resolve_beam_label(Fail), Src, ARG1);
    if (masked_types<BeamTypeId::MaybeBoxed>(Src) == BeamTypeId::Float) {
        comment("skipped header test since we know it's a float when boxed");
    } else {
        x86::Gp boxed_ptr = emit_ptr_val(ARG1, ARG1);
        preserve_cache([&]() {
            a.cmp(emit_boxed_val(boxed_ptr), imm(HEADER_FLONUM));
            a.jne(resolve_beam_label(Fail));
        });
    }
}
void BeamModuleAssembler::emit_is_function(const ArgLabel &Fail,
                                           const ArgRegister &Src) {
    mov_arg(RET, Src);
    emit_is_boxed(resolve_beam_label(Fail), Src, RET);
    if (masked_types<BeamTypeId::MaybeBoxed>(Src) == BeamTypeId::Fun) {
        comment("skipped header test since we know it's a fun when boxed");
    } else {
        x86::Gp boxed_ptr = emit_ptr_val(RET, RET);
        preserve_cache([&]() {
            ERTS_CT_ASSERT(FUN_HEADER_ARITY_OFFS == 8);
            a.cmp(emit_boxed_val(boxed_ptr, 0, sizeof(byte)), imm(FUN_SUBTAG));
            a.jne(resolve_beam_label(Fail));
        });
    }
}
void BeamModuleAssembler::emit_is_function2(const ArgLabel &Fail,
                                            const ArgSource &Src,
                                            const ArgSource &Arity) {
    if (!Arity.isSmall()) {
        mov_arg(ARG2, Src);
        mov_arg(ARG3, Arity);
        emit_enter_runtime();
        a.mov(ARG1, c_p);
        runtime_call<Eterm (*)(Process *, Eterm, Eterm), erl_is_function>();
        emit_leave_runtime();
        a.cmp(RET, imm(am_true));
        a.jne(resolve_beam_label(Fail));
        return;
    }
    unsigned arity = Arity.as<ArgSmall>().getUnsigned();
    if (arity > MAX_ARG) {
        a.jmp(resolve_beam_label(Fail));
        return;
    }
    mov_arg(ARG1, Src);
    emit_is_boxed(resolve_beam_label(Fail), Src, ARG1);
    x86::Gp boxed_ptr = emit_ptr_val(ARG1, ARG1);
    preserve_cache([&]() {
        ERTS_CT_ASSERT(FUN_HEADER_ARITY_OFFS == 8);
        ERTS_CT_ASSERT(FUN_HEADER_ENV_SIZE_OFFS == 16);
        a.cmp(emit_boxed_val(boxed_ptr, 0, sizeof(Uint16)),
              imm(MAKE_FUN_HEADER(arity, 0, 0) & 0xFFFF));
        a.jne(resolve_beam_label(Fail));
    });
}
void BeamModuleAssembler::emit_is_integer(const ArgLabel &Fail,
                                          const ArgSource &Src) {
    if (always_immediate(Src)) {
        comment("skipped test for boxed since the value is always immediate");
        mov_arg(RET, Src);
        preserve_cache(
                [&]() {
                    a.and_(RETb, imm(_TAG_IMMED1_MASK));
                    a.cmp(RETb, imm(_TAG_IMMED1_SMALL));
                    a.jne(resolve_beam_label(Fail));
                },
                RET);
        return;
    }
    Label next = a.new_label();
    mov_arg(ARG1, Src);
    if (always_one_of<BeamTypeId::Integer, BeamTypeId::AlwaysBoxed>(Src)) {
        comment("simplified small test since all other types are boxed");
        emit_is_boxed(next, Src, ARG1);
    } else {
        preserve_cache(
                [&]() {
                    a.mov(RETd, ARG1d);
                    a.and_(RETb, imm(_TAG_IMMED1_MASK));
                    a.cmp(RETb, imm(_TAG_IMMED1_SMALL));
                    a.short_().je(next);
                },
                RET);
        emit_is_boxed(resolve_beam_label(Fail), Src, RET);
    }
    if (masked_types<BeamTypeId::MaybeBoxed>(Src) == BeamTypeId::Integer) {
        comment("skipped header test since we know it's a bignum when "
                "boxed");
    } else {
        x86::Gp boxed_ptr = emit_ptr_val(ARG1, ARG1);
        preserve_cache(
                [&]() {
                    a.mov(RETd, emit_boxed_val(boxed_ptr, 0, sizeof(Uint32)));
                    a.and_(RETb, imm(_BIG_TAG_MASK));
                    a.cmp(RETb, imm(_TAG_HEADER_POS_BIG));
                    a.jne(resolve_beam_label(Fail));
                },
                RET);
    }
    a.bind(next);
}
void BeamModuleAssembler::emit_is_list(const ArgLabel &Fail,
                                       const ArgSource &Src) {
    Label next = a.new_label();
    mov_arg(RET, Src);
    preserve_cache([&]() {
        a.cmp(RET, imm(NIL));
        a.short_().je(next);
    });
    emit_is_cons(resolve_beam_label(Fail), RET);
    a.bind(next);
}
void BeamModuleAssembler::emit_is_map(const ArgLabel &Fail,
                                      const ArgSource &Src) {
    mov_arg(ARG1, Src);
    emit_is_boxed(resolve_beam_label(Fail), Src, ARG1);
    if (masked_types<BeamTypeId::MaybeBoxed>(Src) == BeamTypeId::Map) {
        comment("skipped header test since we know it's a map when boxed");
    } else {
        preserve_cache(
                [&]() {
                    x86::Gp boxed_ptr = emit_ptr_val(RET, ARG1);
                    a.mov(RETd, emit_boxed_val(boxed_ptr, 0, sizeof(Uint32)));
                    a.and_(RETb, imm(_TAG_HEADER_MASK));
                    a.cmp(RETb, imm(_TAG_HEADER_MAP));
                    a.jne(resolve_beam_label(Fail));
                },
                RET);
    }
}
void BeamModuleAssembler::emit_is_nil(const ArgLabel &Fail,
                                      const ArgRegister &Src) {
    preserve_cache([&]() {
        a.cmp(getArgRef(Src, 1), imm(NIL));
        a.jne(resolve_beam_label(Fail));
    });
}
void BeamModuleAssembler::emit_is_number(const ArgLabel &Fail,
                                         const ArgSource &Src) {
    Label fail = resolve_beam_label(Fail);
    Label next = a.new_label();
    mov_arg(ARG1, Src);
    if (always_one_of<BeamTypeId::Integer, BeamTypeId::AlwaysBoxed>(Src)) {
        comment("simplified small test test since all other types are boxed");
        emit_is_boxed(next, Src, ARG1);
    } else {
        a.mov(RETd, ARG1d);
        a.and_(RETb, imm(_TAG_IMMED1_MASK));
        a.cmp(RETb, imm(_TAG_IMMED1_SMALL));
        a.short_().je(next);
        emit_is_boxed(fail, Src, RET);
    }
    if (masked_types<BeamTypeId::MaybeBoxed>(Src) == BeamTypeId::Number) {
        comment("skipped header test since we know it's a number when boxed");
    } else {
        x86::Gp boxed_ptr = emit_ptr_val(ARG1, ARG1);
        a.mov(ARG1, emit_boxed_val(boxed_ptr));
        a.mov(RETd, ARG1d);
        a.and_(RETb, imm(_BIG_TAG_MASK));
        a.cmp(RETb, imm(_TAG_HEADER_POS_BIG));
        a.short_().je(next);
        a.cmp(ARG1d, imm(HEADER_FLONUM));
        a.jne(fail);
    }
    a.bind(next);
}
void BeamModuleAssembler::emit_is_pid(const ArgLabel &Fail,
                                      const ArgSource &Src) {
    Label next = a.new_label();
    mov_arg(ARG1, Src);
    if (always_one_of<BeamTypeId::Pid, BeamTypeId::AlwaysBoxed>(Src)) {
        comment("simplified local pid test since all other types are boxed");
        emit_is_boxed(next, Src, ARG1);
    } else {
        preserve_cache(
                [&]() {
                    a.mov(RETd, ARG1d);
                    a.and_(RETb, imm(_TAG_IMMED1_MASK));
                    a.cmp(RETb, imm(_TAG_IMMED1_PID));
                    a.short_().je(next);
                },
                RET);
        emit_is_boxed(resolve_beam_label(Fail), Src, RET);
    }
    if (masked_types<BeamTypeId::MaybeBoxed>(Src) == BeamTypeId::Pid) {
        comment("skipped header test since we know it's a pid when boxed");
    } else {
        x86::Gp boxed_ptr = emit_ptr_val(ARG1, ARG1);
        preserve_cache(
                [&]() {
                    a.mov(RETd, emit_boxed_val(boxed_ptr, 0, sizeof(Uint32)));
                    a.and_(RETb, imm(_TAG_HEADER_MASK));
                    a.cmp(RETb, imm(_TAG_HEADER_EXTERNAL_PID));
                    a.jne(resolve_beam_label(Fail));
                },
                RET);
    }
    a.bind(next);
}
void BeamModuleAssembler::emit_is_port(const ArgLabel &Fail,
                                       const ArgSource &Src) {
    Label next = a.new_label();
    mov_arg(ARG1, Src);
    if (always_one_of<BeamTypeId::Port, BeamTypeId::AlwaysBoxed>(Src)) {
        comment("simplified local port test since all other types are boxed");
        emit_is_boxed(next, Src, ARG1);
    } else {
        a.mov(RETd, ARG1d);
        a.and_(RETb, imm(_TAG_IMMED1_MASK));
        a.cmp(RETb, imm(_TAG_IMMED1_PORT));
        a.short_().je(next);
        emit_is_boxed(resolve_beam_label(Fail), Src, RET);
    }
    if (masked_types<BeamTypeId::MaybeBoxed>(Src) == BeamTypeId::Port) {
        comment("skipped header test since we know it's a port when boxed");
    } else {
        x86::Gp boxed_ptr = emit_ptr_val(ARG1, ARG1);
        a.mov(RETd, emit_boxed_val(boxed_ptr, 0, sizeof(Uint32)));
        a.and_(RETb, imm(_TAG_HEADER_MASK));
        a.cmp(RETb, imm(_TAG_HEADER_EXTERNAL_PORT));
        a.jne(resolve_beam_label(Fail));
    }
    a.bind(next);
}
void BeamModuleAssembler::emit_is_reference(const ArgLabel &Fail,
                                            const ArgSource &Src) {
    mov_arg(ARG1, Src);
    emit_is_boxed(resolve_beam_label(Fail), Src, ARG1);
    if (masked_types<BeamTypeId::MaybeBoxed>(Src) == BeamTypeId::Reference) {
        comment("skipped header test since we know it's a ref when boxed");
    } else {
        Label next = a.new_label();
        preserve_cache(
                [&]() {
                    x86::Gp boxed_ptr = emit_ptr_val(ARG1, ARG1);
                    a.mov(RETd, emit_boxed_val(boxed_ptr, 0, sizeof(Uint32)));
                    a.and_(RETb, imm(_TAG_HEADER_MASK));
                    a.cmp(RETb, imm(_TAG_HEADER_REF));
                    a.short_().je(next);
                    a.cmp(RETb, imm(_TAG_HEADER_EXTERNAL_REF));
                    a.jne(resolve_beam_label(Fail));
                },
                RET);
        a.bind(next);
    }
}
void BeamModuleAssembler::emit_i_is_tagged_tuple(const ArgLabel &Fail,
                                                 const ArgSource &Src,
                                                 const ArgWord &Arity,
                                                 const ArgAtom &Tag) {
    mov_arg(ARG2, Src);
    emit_is_boxed(resolve_beam_label(Fail), Src, ARG2);
    x86::Gp boxed_ptr = emit_ptr_val(ARG2, ARG2);
    ERTS_CT_ASSERT(Support::is_int_n<32>(make_arityval(MAX_ARITYVAL)));
    preserve_cache([&]() {
        a.cmp(emit_boxed_val(boxed_ptr, 0, sizeof(Uint32)), imm(Arity.get()));
        a.jne(resolve_beam_label(Fail));
        a.cmp(emit_boxed_val(boxed_ptr, sizeof(Eterm)), imm(Tag.get()));
        a.jne(resolve_beam_label(Fail));
    });
}
void BeamModuleAssembler::emit_i_is_tagged_tuple_ff(const ArgLabel &NotTuple,
                                                    const ArgLabel &NotRecord,
                                                    const ArgSource &Src,
                                                    const ArgWord &Arity,
                                                    const ArgAtom &Tag) {
    mov_arg(ARG2, Src);
    emit_is_boxed(resolve_beam_label(NotTuple), Src, ARG2);
    (void)emit_ptr_val(ARG2, ARG2);
    a.mov(ARG1, emit_boxed_val(ARG2));
    ERTS_CT_ASSERT(_TAG_HEADER_ARITYVAL == 0);
    a.test(ARG1.r8(), imm(_TAG_HEADER_MASK));
    a.jne(resolve_beam_label(NotTuple));
    ERTS_CT_ASSERT(Support::is_int_n<32>(make_arityval(MAX_ARITYVAL)));
    a.cmp(ARG1d, imm(Arity.get()));
    a.jne(resolve_beam_label(NotRecord));
    a.cmp(emit_boxed_val(ARG2, sizeof(Eterm)), imm(Tag.get()));
    a.jne(resolve_beam_label(NotRecord));
}
void BeamModuleAssembler::emit_i_is_tuple(const ArgLabel &Fail,
                                          const ArgSource &Src) {
    mov_arg(ARG2, Src);
    if (masked_types<BeamTypeId::MaybeBoxed>(Src) == BeamTypeId::Tuple) {
        comment("simplified tuple test since the source is always a tuple "
                "when boxed");
        (void)emit_ptr_val(ARG2, ARG2);
        preserve_cache([&]() {
            a.test(ARG2.r8(), imm(_TAG_PRIMARY_MASK - TAG_PRIMARY_BOXED));
        });
    } else {
        emit_is_boxed(resolve_beam_label(Fail), Src, ARG2);
        (void)emit_ptr_val(ARG2, ARG2);
        ERTS_CT_ASSERT(_TAG_HEADER_ARITYVAL == 0);
        preserve_cache([&]() {
            a.test(emit_boxed_val(ARG2, 0, sizeof(byte)),
                   imm(_TAG_HEADER_MASK));
        });
    }
    preserve_cache([&]() {
        a.jne(resolve_beam_label(Fail));
    });
}
void BeamModuleAssembler::emit_i_is_tuple_of_arity(const ArgLabel &Fail,
                                                   const ArgSource &Src,
                                                   const ArgWord &Arity) {
    mov_arg(ARG2, Src);
    emit_is_boxed(resolve_beam_label(Fail), Src, ARG2);
    (void)emit_ptr_val(ARG2, ARG2);
    ERTS_CT_ASSERT(Support::is_int_n<32>(make_arityval(MAX_ARITYVAL)));
    preserve_cache([&]() {
        a.cmp(emit_boxed_val(ARG2, 0, sizeof(Uint32)), imm(Arity.get()));
        a.jne(resolve_beam_label(Fail));
    });
}
void BeamModuleAssembler::emit_i_is_tuple_of_arity_ff(const ArgLabel &NotTuple,
                                                      const ArgLabel &BadArity,
                                                      const ArgSource &Src,
                                                      const ArgWord &Arity) {
    mov_arg(ARG2, Src);
    if (masked_types<BeamTypeId::MaybeBoxed>(Src) == BeamTypeId::Tuple) {
        comment("simplified tuple test since the source is always a tuple "
                "when boxed");
        (void)emit_ptr_val(ARG2, ARG2);
        emit_is_boxed(resolve_beam_label(NotTuple), ARG2);
        preserve_cache([&]() {
            ERTS_CT_ASSERT(Support::is_int_n<32>(make_arityval(MAX_ARITYVAL)));
            a.cmp(emit_boxed_val(ARG2, 0, sizeof(Uint32)), imm(Arity.get()));
            a.jne(resolve_beam_label(BadArity));
        });
    } else {
        emit_is_boxed(resolve_beam_label(NotTuple), Src, ARG2);
        (void)emit_ptr_val(ARG2, ARG2);
        ERTS_CT_ASSERT(Support::is_int_n<32>(make_arityval(MAX_ARITYVAL)));
        preserve_cache(
                [&]() {
                    a.mov(RETd, emit_boxed_val(ARG2, 0, sizeof(Uint32)));
                    a.test(RETb, imm(_TAG_HEADER_MASK));
                    a.jne(resolve_beam_label(NotTuple));
                    a.cmp(RETd, imm(Arity.get()));
                    a.jne(resolve_beam_label(BadArity));
                },
                RET);
    }
}
void BeamModuleAssembler::emit_i_test_arity(const ArgLabel &Fail,
                                            const ArgSource &Src,
                                            const ArgWord &Arity) {
    mov_arg(ARG2, Src);
    (void)emit_ptr_val(ARG2, ARG2);
    ERTS_CT_ASSERT(Support::is_int_n<32>(make_arityval(MAX_ARITYVAL)));
    preserve_cache([&]() {
        a.cmp(emit_boxed_val(ARG2, 0, sizeof(Uint32)), imm(Arity.get()));
        a.jne(resolve_beam_label(Fail));
    });
}
void BeamGlobalAssembler::emit_is_eq_exact_list_shared() {
    Label loop = a.new_label(), mid = a.new_label(), done = a.new_label();
    a.short_().jmp(mid);
    a.bind(loop);
    (void)emit_ptr_val(ARG1, ARG1);
    (void)emit_ptr_val(ARG2, ARG2);
    a.mov(RET, getCARRef(ARG1));
    a.mov(ARG1, getCDRRef(ARG1));
    a.cmp(getCARRef(ARG2), RET);
    a.short_().jne(done);
    a.mov(ARG2, getCDRRef(ARG2));
    a.bind(mid);
    a.cmp(ARG1, ARG2);
    a.short_().je(done);
#if !defined(DEBUG)
    ERTS_CT_ASSERT(!is_list(TAG_PRIMARY_LIST | TAG_PRIMARY_BOXED));
    ERTS_CT_ASSERT(!is_list(TAG_PRIMARY_LIST | TAG_PRIMARY_IMMED1));
#endif
    a.mov(RETd, ARG1d);
    a.or_(RETd, ARG2d);
    emit_is_not_cons(loop, RET);
    ERTS_CT_ASSERT(TAG_PRIMARY_HEADER == 0);
    a.cmp(RETb, imm(0));
    a.bind(done);
    a.ret();
}
void BeamGlobalAssembler::emit_is_eq_exact_shallow_boxed_shared() {
    Label loop = a.new_label();
    Label done = a.new_label();
    Label not_equal = a.new_label();
    a.mov(RETd, ARG1d);
    a.or_(RETd, ARG2d);
    emit_is_boxed(not_equal, RET);
    a.and_(ARG1, imm(~TAG_PTR_MASK__));
    a.and_(ARG2, imm(~TAG_PTR_MASK__));
    a.mov(ARG3, x86::qword_ptr(ARG1, 0));
    a.shr(ARG3, imm(_HEADER_ARITY_OFFS));
    a.dec(ARG3);
    mov_imm(ARG4, 0);
    a.bind(loop);
    {
        if (hasCpuFeature(CpuFeatures::X86::kAVX)) {
            a.vmovdqu(x86::xmm0, x86::xmmword_ptr(ARG1, ARG4));
            a.vpxor(x86::xmm0, x86::xmm0, x86::xmmword_ptr(ARG2, ARG4));
            a.vptest(x86::xmm0, x86::xmm0);
        } else {
            a.mov(RET, x86::qword_ptr(ARG1, ARG4));
            a.cmp(RET, x86::qword_ptr(ARG2, ARG4));
            a.short_().jne(done);
            a.mov(RET, x86::qword_ptr(ARG1, ARG4, 0, sizeof(Eterm)));
            a.cmp(RET, x86::qword_ptr(ARG2, ARG4, 0, sizeof(Eterm)));
        }
        a.short_().jne(done);
        a.add(ARG4, imm(2 * sizeof(Eterm)));
        a.sub(ARG3, imm(2));
        a.jge(loop);
    }
    a.cmp(ARG3.r8(), imm(-2));
    a.short_().je(done);
    a.mov(RET, x86::qword_ptr(ARG1, ARG4, 0));
    a.cmp(RET, x86::qword_ptr(ARG2, ARG4, 0));
    a.bind(done);
    a.ret();
    a.bind(not_equal);
    a.cmp(RETb, 0);
    a.ret();
}
void BeamModuleAssembler::emit_is_eq_exact(const ArgLabel &Fail,
                                           const ArgSource &X,
                                           const ArgSource &Y) {
    if (Y.isLiteral()) {
        Eterm literal = beamfile_get_literal(beam, Y.as<ArgLiteral>().get());
        bool imm_list = beam_jit_is_list_of_immediates(literal);
        if (imm_list && erts_list_length(literal) == 1) {
            Sint head = (Sint)CAR(list_val(literal));
            comment("optimized equality test with %T", literal);
            mov_arg(RET, X);
            if (!exact_type<BeamTypeId::Cons>(X)) {
                emit_is_cons(resolve_beam_label(Fail), RET);
            }
            (void)emit_ptr_val(RET, RET);
            if (Support::is_int_n<32>(head)) {
                a.cmp(getCARRef(RET), imm(head));
            } else {
                mov_imm(ARG1, head);
                a.cmp(getCARRef(RET), ARG1);
            }
            a.jne(resolve_beam_label(Fail));
            a.cmp(getCDRRef(RET), imm(NIL));
            a.jne(resolve_beam_label(Fail));
            return;
        } else if (imm_list) {
            comment("optimized equality test with %T", literal);
            mov_arg(ARG2, Y);
            mov_arg(ARG1, X);
            safe_fragment_call(ga->get_is_eq_exact_list_shared());
            a.jne(resolve_beam_label(Fail));
            return;
        } else if (beam_jit_is_shallow_boxed(literal)) {
            comment("optimized equality test with %T", literal);
            mov_arg(ARG2, Y);
            mov_arg(ARG1, X);
            safe_fragment_call(ga->get_is_eq_exact_shallow_boxed_shared());
            a.jne(resolve_beam_label(Fail));
            return;
        } else if (is_bitstring(literal) && bitstring_size(literal) == 0) {
            comment("simplified equality test with empty bitstring");
            mov_arg(ARG2, X);
            emit_is_boxed(resolve_beam_label(Fail), X, ARG2);
            x86::Gp boxed_ptr = emit_ptr_val(ARG2, ARG2);
            ERTS_CT_ASSERT(offsetof(ErlHeapBits, size) == sizeof(Eterm));
            a.mov(ARG1, emit_boxed_val(boxed_ptr, sizeof(Eterm)));
            Label not_sub_bits = a.new_label();
            if (masked_types<BeamTypeId::MaybeBoxed>(X) ==
                BeamTypeId::Bitstring) {
                a.cmp(emit_boxed_val(boxed_ptr), imm(HEADER_SUB_BITS));
            } else {
                a.mov(RETd, emit_boxed_val(boxed_ptr, 0, sizeof(Uint32)));
                a.cmp(RETd, imm(HEADER_SUB_BITS));
            }
            a.short_().jne(not_sub_bits);
            a.mov(ARG1, emit_boxed_val(boxed_ptr, offsetof(ErlSubBits, end)));
            a.sub(ARG1, emit_boxed_val(boxed_ptr, offsetof(ErlSubBits, start)));
            a.bind(not_sub_bits);
            if (masked_types<BeamTypeId::MaybeBoxed>(X) ==
                BeamTypeId::Bitstring) {
                comment("skipped header test since we know it's a bitstring "
                        "when boxed");
                a.test(ARG1, ARG1);
            } else {
                a.and_(RETd, imm(_BITSTRING_TAG_MASK));
                a.sub(RETd, imm(_TAG_HEADER_HEAP_BITS));
                a.or_(RETd, ARG1d);
            }
            a.jne(resolve_beam_label(Fail));
            return;
        } else if (is_map(literal) && erts_map_size(literal) == 0) {
            comment("optimized equality test with empty map", literal);
            mov_arg(ARG1, X);
            emit_is_boxed(resolve_beam_label(Fail), X, ARG1);
            (void)emit_ptr_val(ARG1, ARG1);
            a.cmp(emit_boxed_val(ARG1, 0, sizeof(Uint32)), MAP_HEADER_FLATMAP);
            a.jne(resolve_beam_label(Fail));
            a.cmp(emit_boxed_val(ARG1, sizeof(Eterm), sizeof(Uint32)), imm(0));
            a.jne(resolve_beam_label(Fail));
            return;
        }
    }
    if (X.isRegister() && always_immediate(Y)) {
        comment("simplified check since one argument is an immediate");
        cmp_arg(getArgRef(X), Y);
        preserve_cache([&]() {
            a.jne(resolve_beam_label(Fail));
        });
        return;
    }
    Label next = a.new_label();
    mov_arg(ARG2, Y);
    mov_arg(ARG1, X);
    a.cmp(ARG1, ARG2);
#ifdef JIT_HARD_DEBUG
    a.je(next);
#else
    a.short_().je(next);
#endif
    if (exact_type<BeamTypeId::Integer>(X) &&
        exact_type<BeamTypeId::Integer>(Y)) {
        a.mov(RETd, ARG1d);
        a.or_(RETd, ARG2d);
        emit_test_boxed(RET);
        a.jne(resolve_beam_label(Fail));
    } else if (always_same_types(X, Y)) {
        comment("skipped tag test since they are always equal");
    } else {
        emit_is_unequal_based_on_tags(resolve_beam_label(Fail),
                                      X,
                                      ARG1,
                                      Y,
                                      ARG2);
    }
    if (always_one_of<BeamTypeId::Integer, BeamTypeId::Float>(X) ||
        always_one_of<BeamTypeId::Integer, BeamTypeId::Float>(Y)) {
        safe_fragment_call(ga->get_is_eq_exact_shallow_boxed_shared());
        a.jne(resolve_beam_label(Fail));
    } else {
        emit_enter_runtime();
        runtime_call<int (*)(Eterm, Eterm), eq>();
        emit_leave_runtime();
        a.test(RETd, RETd);
        a.je(resolve_beam_label(Fail));
    }
    a.bind(next);
}
void BeamModuleAssembler::emit_is_ne_exact(const ArgLabel &Fail,
                                           const ArgSource &X,
                                           const ArgSource &Y) {
    if (Y.isLiteral()) {
        Eterm literal = beamfile_get_literal(beam, Y.as<ArgLiteral>().get());
        bool imm_list = beam_jit_is_list_of_immediates(literal);
        if (imm_list && erts_list_length(literal) == 1) {
            Sint head = (Sint)CAR(list_val(literal));
            Label next = a.new_label();
            comment("optimized non-equality test with %T", literal);
            mov_arg(RET, X);
            if (!exact_type<BeamTypeId::Cons>(X)) {
                emit_is_cons(next, RET, dShort);
            }
            (void)emit_ptr_val(RET, RET);
            if (Support::is_int_n<32>(head)) {
                a.cmp(getCARRef(RET), imm(head));
            } else {
                mov_imm(ARG1, head);
                a.cmp(getCARRef(RET), ARG1);
            }
            a.short_().jne(next);
            a.cmp(getCDRRef(RET), imm(NIL));
            a.je(resolve_beam_label(Fail));
            a.bind(next);
            return;
        } else if (imm_list) {
            comment("optimized non-equality test with %T", literal);
            mov_arg(ARG2, Y);
            mov_arg(ARG1, X);
            safe_fragment_call(ga->get_is_eq_exact_list_shared());
            a.je(resolve_beam_label(Fail));
            return;
        } else if (beam_jit_is_shallow_boxed(literal)) {
            comment("optimized non-equality test with %T", literal);
            mov_arg(ARG2, Y);
            mov_arg(ARG1, X);
            safe_fragment_call(ga->get_is_eq_exact_shallow_boxed_shared());
            a.je(resolve_beam_label(Fail));
            return;
        } else if (is_bitstring(literal) && bitstring_size(literal) == 0) {
            Label next = a.new_label();
            comment("simplified non-equality test with empty bitstring");
            mov_arg(ARG2, X);
            emit_is_boxed(next, X, ARG2, dShort);
            x86::Gp boxed_ptr = emit_ptr_val(ARG2, ARG2);
            ERTS_CT_ASSERT(offsetof(ErlHeapBits, size) == sizeof(Eterm));
            a.mov(ARG1, emit_boxed_val(boxed_ptr, sizeof(Eterm)));
            Label not_sub_bits = a.new_label();
            if (masked_types<BeamTypeId::MaybeBoxed>(X) ==
                BeamTypeId::Bitstring) {
                a.cmp(emit_boxed_val(boxed_ptr), imm(HEADER_SUB_BITS));
            } else {
                a.mov(RETd, emit_boxed_val(boxed_ptr, 0, sizeof(Uint32)));
                a.cmp(RETd, imm(HEADER_SUB_BITS));
            }
            a.short_().jne(not_sub_bits);
            a.mov(ARG1, emit_boxed_val(boxed_ptr, offsetof(ErlSubBits, end)));
            a.sub(ARG1, emit_boxed_val(boxed_ptr, offsetof(ErlSubBits, start)));
            a.bind(not_sub_bits);
            if (masked_types<BeamTypeId::MaybeBoxed>(X) ==
                BeamTypeId::Bitstring) {
                comment("skipped header test since we know it's a bitstring "
                        "when boxed");
                a.test(ARG1, ARG1);
            } else {
                a.and_(RETd, imm(_BITSTRING_TAG_MASK));
                a.sub(RETd, imm(_TAG_HEADER_HEAP_BITS));
                a.or_(RETd, ARG1d);
            }
            a.je(resolve_beam_label(Fail));
            a.bind(next);
            return;
        } else if (is_map(literal) && erts_map_size(literal) == 0) {
            Label next = a.new_label();
            comment("optimized non-equality test with empty map", literal);
            mov_arg(ARG1, X);
            emit_is_boxed(next, X, ARG1, dShort);
            (void)emit_ptr_val(ARG1, ARG1);
            a.cmp(emit_boxed_val(ARG1, 0, sizeof(Uint32)), MAP_HEADER_FLATMAP);
            a.short_().jne(next);
            a.cmp(emit_boxed_val(ARG1, sizeof(Eterm), sizeof(Uint32)), imm(0));
            a.je(resolve_beam_label(Fail));
            a.bind(next);
            return;
        }
    }
    if (X.isRegister() && always_immediate(Y)) {
        comment("simplified check since one argument is an immediate");
        cmp_arg(getArgRef(X), Y);
        preserve_cache([&]() {
            a.je(resolve_beam_label(Fail));
        });
        return;
    }
    Label next = a.new_label();
    mov_arg(ARG2, Y);
    mov_arg(ARG1, X);
    a.cmp(ARG1, ARG2);
    a.je(resolve_beam_label(Fail));
    if (exact_type<BeamTypeId::Integer>(X) &&
        exact_type<BeamTypeId::Integer>(Y)) {
        a.mov(RETd, ARG1d);
        a.or_(RETd, ARG2d);
        emit_test_boxed(RET);
        a.short_().jne(next);
    } else if (always_same_types(X, Y)) {
        comment("skipped tag test since they are always equal");
    } else {
#ifdef JIT_HARD_DEBUG
        emit_is_unequal_based_on_tags(next, X, ARG1, Y, ARG2);
#else
        emit_is_unequal_based_on_tags(next, X, ARG1, Y, ARG2, dShort);
#endif
    }
    if (always_one_of<BeamTypeId::Integer, BeamTypeId::Float>(X) ||
        always_one_of<BeamTypeId::Integer, BeamTypeId::Float>(Y)) {
        safe_fragment_call(ga->get_is_eq_exact_shallow_boxed_shared());
        a.jz(resolve_beam_label(Fail));
    } else {
        emit_enter_runtime();
        runtime_call<int (*)(Eterm, Eterm), eq>();
        emit_leave_runtime();
        a.test(RETd, RETd);
        a.jnz(resolve_beam_label(Fail));
    }
    a.bind(next);
}
void BeamGlobalAssembler::emit_arith_eq_shared() {
    Label generic_compare = a.new_label();
    a.mov(ARG3d, ARG1d);
    a.or_(ARG3d, ARG2d);
    a.and_(ARG3d, imm(_TAG_PRIMARY_MASK - TAG_PRIMARY_BOXED));
    a.short_().jne(generic_compare);
    x86::Gp boxed_ptr = emit_ptr_val(ARG3, ARG1);
    a.mov(ARG3, emit_boxed_val(boxed_ptr));
    boxed_ptr = emit_ptr_val(ARG5, ARG2);
    a.mov(ARG5, emit_boxed_val(boxed_ptr));
    a.and_(ARG3d, imm(_TAG_HEADER_MASK));
    a.and_(ARG5d, imm(_TAG_HEADER_MASK));
    a.sub(ARG3d, imm(_TAG_HEADER_FLOAT));
    a.sub(ARG5d, imm(_TAG_HEADER_FLOAT));
    a.or_(ARG3d, ARG5d);
    a.short_().jne(generic_compare);
    boxed_ptr = emit_ptr_val(ARG1, ARG1);
    vmovsd(x86::xmm0, emit_boxed_val(boxed_ptr, sizeof(Eterm)));
    boxed_ptr = emit_ptr_val(ARG2, ARG2);
    vmovsd(x86::xmm1, emit_boxed_val(boxed_ptr, sizeof(Eterm)));
    vucomisd(x86::xmm0, x86::xmm1);
    a.ret();
    a.bind(generic_compare);
    {
        emit_enter_runtime();
        comment("erts_cmp_compound(X, Y, 0, 1);");
        mov_imm(ARG3, 0);
        mov_imm(ARG4, 1);
        runtime_call<Sint (*)(Eterm, Eterm, int, int), erts_cmp_compound>();
        emit_leave_runtime();
        a.test(RET, RET);
        a.ret();
    }
}
void BeamModuleAssembler::emit_is_eq(const ArgLabel &Fail,
                                     const ArgSource &A,
                                     const ArgSource &B) {
    Label fail = resolve_beam_label(Fail), next = a.new_label();
    mov_arg(ARG2, B);
    mov_arg(ARG1, A);
    a.cmp(ARG1, ARG2);
    a.short_().je(next);
    if (always_one_of<BeamTypeId::Cons, BeamTypeId::AlwaysBoxed>(A) ||
        always_one_of<BeamTypeId::Cons, BeamTypeId::AlwaysBoxed>(B)) {
        comment("skipped test for immediate because one operand never is");
    } else {
        emit_are_both_immediate(A, ARG1, B, ARG2);
        a.je(fail);
    }
    safe_fragment_call(ga->get_arith_eq_shared());
    a.jne(fail);
    a.bind(next);
}
void BeamModuleAssembler::emit_is_ne(const ArgLabel &Fail,
                                     const ArgSource &A,
                                     const ArgSource &B) {
    Label fail = resolve_beam_label(Fail), next = a.new_label();
    mov_arg(ARG2, B);
    mov_arg(ARG1, A);
    a.cmp(ARG1, ARG2);
    a.je(fail);
    if (always_one_of<BeamTypeId::Cons, BeamTypeId::AlwaysBoxed>(A) ||
        always_one_of<BeamTypeId::Cons, BeamTypeId::AlwaysBoxed>(B)) {
        comment("skipped test for immediate because one operand never is");
    } else {
        emit_are_both_immediate(A, ARG1, B, ARG2);
        a.short_().je(next);
    }
    safe_fragment_call(ga->get_arith_eq_shared());
    a.je(fail);
    a.bind(next);
}
void BeamGlobalAssembler::emit_arith_compare_shared() {
    Label atom_compare, generic_compare;
    atom_compare = a.new_label();
    generic_compare = a.new_label();
    emit_enter_frame();
    a.mov(ARG3d, ARG1d);
    a.or_(ARG3d, ARG2d);
    a.and_(ARG3d, imm(_TAG_PRIMARY_MASK - TAG_PRIMARY_BOXED));
    a.short_().jne(atom_compare);
    x86::Gp boxed_ptr = emit_ptr_val(ARG3, ARG1);
    a.mov(ARG3, emit_boxed_val(boxed_ptr));
    boxed_ptr = emit_ptr_val(ARG5, ARG2);
    a.mov(ARG5, emit_boxed_val(boxed_ptr));
    a.and_(ARG3d, imm(_TAG_HEADER_MASK));
    a.and_(ARG5d, imm(_TAG_HEADER_MASK));
    a.sub(ARG3d, imm(_TAG_HEADER_FLOAT));
    a.sub(ARG5d, imm(_TAG_HEADER_FLOAT));
    a.or_(ARG3d, ARG5d);
    a.jne(generic_compare);
    boxed_ptr = emit_ptr_val(ARG1, ARG1);
    vmovsd(x86::xmm0, emit_boxed_val(boxed_ptr, sizeof(Eterm)));
    boxed_ptr = emit_ptr_val(ARG2, ARG2);
    vmovsd(x86::xmm1, emit_boxed_val(boxed_ptr, sizeof(Eterm)));
    vucomisd(x86::xmm0, x86::xmm1);
    a.seta(x86::al);
    a.setb(x86::ah);
    a.sub(x86::al, x86::ah);
    emit_leave_frame();
    a.ret();
    a.bind(atom_compare);
    {
        a.mov(ARG3d, ARG1d);
        a.mov(ARG5d, ARG2d);
        a.and_(ARG3d, imm(_TAG_IMMED2_MASK));
        a.and_(ARG5d, imm(_TAG_IMMED2_MASK));
        a.sub(ARG3d, imm(_TAG_IMMED2_ATOM));
        a.sub(ARG5d, imm(_TAG_IMMED2_ATOM));
        a.or_(ARG3d, ARG5d);
        a.jne(generic_compare);
        emit_enter_runtime();
        runtime_call<int (*)(Eterm, Eterm), erts_cmp_atoms>();
        emit_leave_runtime();
        a.test(RETd, RETd);
        emit_leave_frame();
        a.ret();
    }
    a.bind(generic_compare);
    {
        emit_enter_runtime();
        comment("erts_cmp_compound(X, Y, 0, 0);");
        mov_imm(ARG3, 0);
        mov_imm(ARG4, 0);
        runtime_call<Sint (*)(Eterm, Eterm, int, int), erts_cmp_compound>();
        emit_leave_runtime();
        a.test(RET, RET);
        emit_leave_frame();
        a.ret();
    }
}
void BeamModuleAssembler::emit_is_lt(const ArgLabel &Fail,
                                     const ArgSource &LHS,
                                     const ArgSource &RHS) {
    Label generic = a.new_label(), do_jge = a.new_label(), next = a.new_label();
    bool both_small = always_small(LHS) && always_small(RHS);
    bool need_generic = !both_small;
    bool never_small = LHS.isLiteral() || RHS.isLiteral();
    mov_arg(ARG2, RHS);
    mov_arg(ARG1, LHS);
    if (both_small) {
        comment("skipped test for small operands since they are always small");
    } else if (always_small(LHS) && exact_type<BeamTypeId::Integer>(RHS) &&
               hasLowerBound(RHS)) {
        comment("simplified test because it always succeeds when RHS is a "
                "bignum");
        need_generic = false;
        emit_is_not_boxed(next, ARG2, dShort);
    } else if (always_small(LHS) && exact_type<BeamTypeId::Integer>(RHS) &&
               hasUpperBound(RHS)) {
        comment("simplified test because it always fails when RHS is a bignum");
        need_generic = false;
        emit_is_not_boxed(resolve_beam_label(Fail), ARG2);
    } else if (exact_type<BeamTypeId::Integer>(LHS) && hasLowerBound(LHS) &&
               always_small(RHS)) {
        comment("simplified test because it always fails when LHS is a bignum");
        need_generic = false;
        emit_is_not_boxed(resolve_beam_label(Fail), ARG1);
    } else if (exact_type<BeamTypeId::Integer>(LHS) && hasUpperBound(LHS) &&
               always_small(RHS)) {
        comment("simplified test because it always succeeds when LHS is a "
                "bignum");
        emit_is_not_boxed(next, ARG1, dShort);
    } else if (never_small) {
        comment("skipped test for small because one operand is never small");
        a.cmp(ARG1, ARG2);
        a.short_().je(do_jge);
    } else if (always_one_of<BeamTypeId::Integer, BeamTypeId::AlwaysBoxed>(
                       LHS) &&
               always_one_of<BeamTypeId::Integer, BeamTypeId::AlwaysBoxed>(
                       RHS)) {
        comment("simplified small test since all other types are boxed");
        if (always_small(LHS)) {
            emit_is_not_boxed(generic, ARG2, dShort);
        } else if (always_small(RHS)) {
            emit_is_not_boxed(generic, ARG1, dShort);
        } else {
            a.mov(RETd, ARG1d);
            a.and_(RETd, ARG2d);
            emit_is_not_boxed(generic, RET, dShort);
        }
    } else {
        if (always_small(RHS)) {
            a.mov(RETd, ARG1d);
        } else if (always_small(LHS)) {
            a.mov(RETd, ARG2d);
        } else {
            a.cmp(ARG1, ARG2);
            a.short_().je(do_jge);
            a.mov(RETd, ARG1d);
            a.and_(RETd, ARG2d);
        }
        a.and_(RETb, imm(_TAG_IMMED1_MASK));
        a.cmp(RETb, imm(_TAG_IMMED1_SMALL));
        a.short_().jne(generic);
    }
    if (!never_small) {
        a.cmp(ARG1, ARG2);
        if (need_generic) {
            a.short_().jmp(do_jge);
        }
    }
    a.bind(generic);
    {
        if (need_generic) {
            safe_fragment_call(ga->get_arith_compare_shared());
        }
    }
    a.bind(do_jge);
    a.jge(resolve_beam_label(Fail));
    a.bind(next);
}
void BeamModuleAssembler::emit_is_ge(const ArgLabel &Fail,
                                     const ArgSource &LHS,
                                     const ArgSource &RHS) {
    bool both_small = always_small(LHS) && always_small(RHS);
    if (both_small && LHS.isRegister() && RHS.isImmed() &&
        Support::is_int_n<32>(RHS.as<ArgImmed>().get())) {
        comment("simplified compare because one operand is an immediate small");
        preserve_cache([&]() {
            a.cmp(getArgRef(LHS.as<ArgRegister>()),
                  imm(RHS.as<ArgImmed>().get()));
            a.jl(resolve_beam_label(Fail));
        });
        return;
    } else if (both_small && RHS.isRegister() && LHS.isImmed() &&
               Support::is_int_n<32>(LHS.as<ArgImmed>().get())) {
        comment("simplified compare because one operand is an immediate small");
        preserve_cache([&]() {
            a.cmp(getArgRef(RHS.as<ArgRegister>()),
                  imm(LHS.as<ArgImmed>().get()));
            a.jg(resolve_beam_label(Fail));
        });
        return;
    }
    Label generic = a.new_label(), small = a.new_label(), do_jl = a.new_label(),
          next = a.new_label();
    bool need_generic = !both_small;
    bool never_small = LHS.isLiteral() || RHS.isLiteral();
    mov_arg(ARG2, RHS);
    mov_arg(ARG1, LHS);
    if (both_small) {
        comment("skipped test for small operands since they are always small");
    } else if (always_small(LHS) && exact_type<BeamTypeId::Integer>(RHS) &&
               hasLowerBound(RHS)) {
        comment("simplified test because it always fails when RHS is a bignum");
        need_generic = false;
        emit_is_not_boxed(resolve_beam_label(Fail), ARG2);
    } else if (always_small(LHS) && exact_type<BeamTypeId::Integer>(RHS) &&
               hasUpperBound(RHS)) {
        comment("simplified test because it always succeeds when RHS is a "
                "bignum");
        need_generic = false;
        emit_is_not_boxed(next, ARG2, dShort);
    } else if (exact_type<BeamTypeId::Integer>(LHS) && hasUpperBound(LHS) &&
               always_small(RHS)) {
        comment("simplified test because it always fails when LHS is a bignum");
        need_generic = false;
        emit_is_not_boxed(resolve_beam_label(Fail), ARG1);
    } else if (exact_type<BeamTypeId::Integer>(LHS) && hasLowerBound(LHS) &&
               always_small(RHS)) {
        comment("simplified test because it always succeeds when LHS is a "
                "bignum");
        need_generic = false;
        emit_is_not_boxed(next, ARG1, dShort);
    } else if (exact_type<BeamTypeId::Integer>(LHS) && always_small(RHS)) {
        x86::Gp boxed_ptr;
        int sign_bit = NEG_BIG_SUBTAG - POS_BIG_SUBTAG;
        ERTS_CT_ASSERT(NEG_BIG_SUBTAG > POS_BIG_SUBTAG);
        comment("simplified small test for known integer");
        need_generic = false;
        emit_is_boxed(small, ARG1, dShort);
        boxed_ptr = emit_ptr_val(ARG1, ARG1);
        a.mov(RETd, emit_boxed_val(boxed_ptr, 0, sizeof(Uint32)));
        a.test(RETb, imm(sign_bit));
        a.jne(resolve_beam_label(Fail));
        a.short_().jmp(next);
    } else if (always_small(LHS) && exact_type<BeamTypeId::Integer>(RHS)) {
        x86::Gp boxed_ptr;
        comment("simplified small test for known integer");
        need_generic = false;
        emit_is_boxed(small, ARG2, dShort);
        boxed_ptr = emit_ptr_val(ARG2, ARG2);
        a.mov(RETd, emit_boxed_val(boxed_ptr, 0, sizeof(Uint32)));
        a.and_(RETb, imm(_TAG_HEADER_MASK));
        ERTS_CT_ASSERT(_TAG_HEADER_NEG_BIG > _TAG_HEADER_POS_BIG);
        a.cmp(RETb, imm(_TAG_HEADER_NEG_BIG));
        a.short_().jmp(do_jl);
    } else if (never_small) {
        comment("skipped test for small because one operand is never small");
        a.cmp(ARG1, ARG2);
        a.short_().je(next);
    } else if (always_one_of<BeamTypeId::Integer, BeamTypeId::AlwaysBoxed>(
                       LHS) &&
               always_one_of<BeamTypeId::Integer, BeamTypeId::AlwaysBoxed>(
                       RHS)) {
        comment("simplified small test since all other types are boxed");
        if (always_small(LHS)) {
            emit_is_not_boxed(generic, ARG2, dShort);
        } else if (always_small(RHS)) {
            emit_is_not_boxed(generic, ARG1, dShort);
        } else {
            a.mov(RETd, ARG1d);
            a.and_(RETd, ARG2d);
            emit_is_not_boxed(generic, RET, dShort);
        }
    } else {
        if (always_small(RHS)) {
            a.mov(RETd, ARG1d);
        } else if (always_small(LHS)) {
            a.mov(RETd, ARG2d);
        } else {
            a.cmp(ARG1, ARG2);
            a.short_().je(next);
            a.mov(RETd, ARG1d);
            a.and_(RETd, ARG2d);
        }
        a.and_(RETb, imm(_TAG_IMMED1_MASK));
        a.cmp(RETb, imm(_TAG_IMMED1_SMALL));
        a.short_().jne(generic);
    }
    a.bind(small);
    if (!never_small) {
        cmp_preserve_cache(ARG1, ARG2);
        if (need_generic) {
            a.short_().jmp(do_jl);
        }
    }
    a.bind(generic);
    {
        if (need_generic) {
            safe_fragment_call(ga->get_arith_compare_shared());
        }
    }
    a.bind(do_jl);
    preserve_cache([&]() {
        a.jl(resolve_beam_label(Fail));
    });
    a.bind(next);
}
void BeamGlobalAssembler::emit_is_in_range_shared() {
    Label immediate = a.new_label();
    Label generic_compare = a.new_label();
    Label done = a.new_label();
    emit_is_boxed(immediate, ARG1);
    x86::Gp boxed_ptr = emit_ptr_val(ARG4, ARG1);
    a.cmp(emit_boxed_val(boxed_ptr), imm(HEADER_FLONUM));
    a.short_().jne(generic_compare);
    vmovsd(x86::xmm0, emit_boxed_val(boxed_ptr, sizeof(Eterm)));
    a.sar(ARG2, imm(_TAG_IMMED1_SIZE));
    a.sar(ARG3, imm(_TAG_IMMED1_SIZE));
    if (hasCpuFeature(CpuFeatures::X86::kAVX)) {
        a.vcvtsi2sd(x86::xmm1, x86::xmm1, ARG2);
        a.vcvtsi2sd(x86::xmm2, x86::xmm2, ARG3);
    } else {
        a.cvtsi2sd(x86::xmm1, ARG2);
        a.cvtsi2sd(x86::xmm2, ARG3);
    }
    mov_imm(RET, -1);
    mov_imm(x86::rcx, 0);
    vucomisd(x86::xmm0, x86::xmm2);
    a.seta(x86::cl);
    vucomisd(x86::xmm1, x86::xmm0);
    a.cmovbe(RET, x86::rcx);
    a.cmp(RET, imm(0));
    a.ret();
    a.bind(immediate);
    {
        mov_imm(RET, 1);
        a.cmp(RET, imm(0));
        a.ret();
    }
    a.bind(generic_compare);
    {
        emit_enter_runtime();
        a.mov(TMP_MEM1q, ARG1);
        a.mov(TMP_MEM2q, ARG3);
        comment("erts_cmp_compound(X, Y, 0, 0);");
        mov_imm(ARG3, 0);
        mov_imm(ARG4, 0);
        runtime_call<Sint (*)(Eterm, Eterm, int, int), erts_cmp_compound>();
        a.test(RET, RET);
        a.js(done);
        a.mov(ARG1, TMP_MEM1q);
        a.mov(ARG2, TMP_MEM2q);
        comment("erts_cmp_compound(X, Y, 0, 0);");
        mov_imm(ARG3, 0);
        mov_imm(ARG4, 0);
        runtime_call<Sint (*)(Eterm, Eterm, int, int), erts_cmp_compound>();
        a.test(RET, RET);
        a.bind(done);
        emit_leave_runtime();
        a.ret();
    }
}
void BeamModuleAssembler::emit_is_in_range(ArgLabel const &Small,
                                           ArgLabel const &Large,
                                           ArgRegister const &Src,
                                           ArgConstant const &Min,
                                           ArgConstant const &Max) {
    Label next = a.new_label(), generic = a.new_label();
    bool need_generic = true;
    mov_arg(ARG1, Src);
    if (always_small(Src)) {
        need_generic = false;
        comment("skipped test for small operand since it always small");
    } else if (always_one_of<BeamTypeId::Integer, BeamTypeId::AlwaysBoxed>(
                       Src)) {
        comment("simplified small test since all other types are boxed");
        ERTS_CT_ASSERT(_TAG_PRIMARY_MASK - TAG_PRIMARY_BOXED == (1 << 0));
        if (Small == Large && never_one_of<BeamTypeId::Float>(Src)) {
            need_generic = false;
            preserve_cache([&]() {
                a.test(ARG1.r8(), imm(_TAG_PRIMARY_MASK - TAG_PRIMARY_BOXED));
                a.je(resolve_beam_label(Small));
            });
        } else {
            a.test(ARG1.r8(), imm(_TAG_PRIMARY_MASK - TAG_PRIMARY_BOXED));
            a.short_().je(generic);
        }
    } else if (Small == Large) {
        ERTS_CT_ASSERT(_TAG_IMMED1_SMALL == _TAG_IMMED1_MASK);
        comment("simplified small & range tests since failure labels are "
                "equal");
        a.mov(RET, ARG1);
        sub(RET, Min.as<ArgImmed>().get(), ARG4);
        a.test(RETb, imm(_TAG_IMMED1_MASK));
        a.jne(generic);
        cmp(RET, Max.as<ArgImmed>().get() - Min.as<ArgImmed>().get(), ARG4);
        a.ja(resolve_beam_label(Small));
        goto test_done;
    } else {
        a.mov(RETd, ARG1d);
        a.and_(RETb, imm(_TAG_IMMED1_MASK));
        a.cmp(RETb, imm(_TAG_IMMED1_SMALL));
        a.short_().jne(generic);
    }
    if (Small == Large) {
        comment("simplified range test since failure labels are equal");
        preserve_cache(
                [&]() {
                    sub(ARG1, Min.as<ArgImmed>().get(), RET);
                    cmp(ARG1,
                        Max.as<ArgImmed>().get() - Min.as<ArgImmed>().get(),
                        RET);
                    a.ja(resolve_beam_label(Small));
                },
                ARG1);
    } else {
        preserve_cache([&]() {
            cmp(ARG1, Min.as<ArgImmed>().get(), RET);
            a.jl(resolve_beam_label(Small));
            cmp(ARG1, Max.as<ArgImmed>().get(), RET);
            a.jg(resolve_beam_label(Large));
        });
    }
test_done:
    if (need_generic) {
        a.short_().jmp(next);
    }
    a.bind(generic);
    if (!need_generic) {
        comment("skipped generic comparison because it is not needed");
    } else {
        mov_arg(ARG2, Min);
        mov_arg(ARG3, Max);
        safe_fragment_call(ga->get_is_in_range_shared());
        if (Small == Large) {
            a.jne(resolve_beam_label(Small));
        } else {
            a.jl(resolve_beam_label(Small));
            a.jg(resolve_beam_label(Large));
        }
    }
    a.bind(next);
}
void BeamGlobalAssembler::emit_is_ge_lt_shared() {
    Label done = a.new_label();
    emit_enter_runtime();
    a.mov(TMP_MEM1q, ARG1);
    a.mov(TMP_MEM2q, ARG3);
    comment("erts_cmp_compound(Src, A, 0, 0);");
    mov_imm(ARG3, 0);
    mov_imm(ARG4, 0);
    runtime_call<Sint (*)(Eterm, Eterm, int, int), erts_cmp_compound>();
    a.test(RET, RET);
    a.short_().js(done);
    comment("erts_cmp_compound(B, Src, 0, 0);");
    a.mov(ARG1, TMP_MEM2q);
    a.mov(ARG2, TMP_MEM1q);
    mov_imm(ARG3, 0);
    mov_imm(ARG4, 0);
    runtime_call<Sint (*)(Eterm, Eterm, int, int), erts_cmp_compound>();
    mov_imm(ARG1, -1);
    mov_imm(ARG4, 1);
    a.test(RET, RET);
    a.cmovs(RET, ARG1);
    a.cmovg(RET, ARG4);
    a.add(RET, imm(1));
    a.bind(done);
    emit_leave_runtime();
    a.ret();
}
void BeamModuleAssembler::emit_is_ge_lt(ArgLabel const &Fail1,
                                        ArgLabel const &Fail2,
                                        ArgRegister const &Src,
                                        ArgConstant const &A,
                                        ArgConstant const &B) {
    Label generic = a.new_label(), next = a.new_label();
    mov_arg(ARG2, A);
    mov_arg(ARG3, B);
    mov_arg(ARG1, Src);
    a.mov(RETd, ARG1d);
    a.and_(RETb, imm(_TAG_IMMED1_MASK));
    a.cmp(RETb, imm(_TAG_IMMED1_SMALL));
    a.short_().jne(generic);
    a.cmp(ARG1, ARG2);
    a.jl(resolve_beam_label(Fail1));
    a.cmp(ARG3, ARG1);
    a.jge(resolve_beam_label(Fail2));
    a.short_().jmp(next);
    a.bind(generic);
    safe_fragment_call(ga->get_is_ge_lt_shared());
    a.jl(resolve_beam_label(Fail1));
    a.jg(resolve_beam_label(Fail2));
    a.bind(next);
}
void BeamModuleAssembler::emit_is_ge_ge(ArgLabel const &Fail1,
                                        ArgLabel const &Fail2,
                                        ArgRegister const &Src,
                                        ArgConstant const &A,
                                        ArgConstant const &B) {
    if (!always_small(Src)) {
        emit_is_ge(Fail1, Src, A);
        emit_is_ge(Fail2, Src, B);
        return;
    }
    mov_arg(RET, Src);
    sub(RET, A.as<ArgImmed>().get(), ARG1);
    preserve_cache([&]() {
        a.jl(resolve_beam_label(Fail1));
    });
    cmp(RET, B.as<ArgImmed>().get() - A.as<ArgImmed>().get(), ARG1);
    preserve_cache([&]() {
        a.jb(resolve_beam_label(Fail2));
    });
}
void BeamModuleAssembler::emit_is_int_in_range(ArgLabel const &Fail,
                                               ArgRegister const &Src,
                                               ArgConstant const &Min,
                                               ArgConstant const &Max) {
    mov_arg(RET, Src);
    sub(RET, Min.as<ArgImmed>().get(), ARG1);
    preserve_cache(
            [&]() {
                ERTS_CT_ASSERT(_TAG_IMMED1_SMALL == _TAG_IMMED1_MASK);
                a.test(RETb, imm(_TAG_IMMED1_MASK));
                a.jne(resolve_beam_label(Fail));
            },
            RET);
    cmp(RET, Max.as<ArgImmed>().get() - Min.as<ArgImmed>().get(), ARG1);
    preserve_cache([&]() {
        a.ja(resolve_beam_label(Fail));
    });
}
void BeamModuleAssembler::emit_is_int_ge(ArgLabel const &Fail,
                                         ArgRegister const &Src,
                                         ArgConstant const &Min) {
    Label small = a.new_label();
    Label fail = a.new_label();
    Label next = a.new_label();
    const x86::Gp src_reg = x86::rcx;
    mov_arg(src_reg, Src);
    if (always_one_of<BeamTypeId::Integer, BeamTypeId::AlwaysBoxed>(Src)) {
        comment("simplified small test since all other types are boxed");
        emit_is_boxed(small, Src, src_reg);
    } else {
        preserve_cache(
                [&]() {
                    a.mov(RETd, src_reg.r32());
                    a.and_(RETb, imm(_TAG_IMMED1_MASK));
                    a.cmp(RETb, imm(_TAG_IMMED1_SMALL));
                    a.short_().je(small);
                },
                RET);
        emit_is_boxed(resolve_beam_label(Fail), Src, src_reg);
    }
    x86::Gp boxed_ptr = emit_ptr_val(src_reg, src_reg);
    preserve_cache(
            [&]() {
                a.mov(RETd, emit_boxed_val(boxed_ptr, 0, sizeof(Uint32)));
                a.and_(RETb, imm(_TAG_HEADER_MASK));
                a.cmp(RETb, imm(_TAG_HEADER_POS_BIG));
                a.short_().je(next);
                a.bind(fail);
                a.jmp(resolve_beam_label(Fail));
                a.bind(small);
                cmp(src_reg, Min.as<ArgImmed>().get(), RET);
                a.short_().jl(fail);
            },
            RET);
    a.bind(next);
}
void BeamModuleAssembler::emit_badmatch(const ArgSource &Src) {
    mov_arg(x86::qword_ptr(c_p, offsetof(Process, fvalue)), Src);
    emit_error(BADMATCH);
}
void BeamModuleAssembler::emit_case_end(const ArgSource &Src) {
    mov_arg(x86::qword_ptr(c_p, offsetof(Process, fvalue)), Src);
    emit_error(EXC_CASE_CLAUSE);
}
void BeamModuleAssembler::emit_system_limit_body() {
    emit_error(SYSTEM_LIMIT);
}
void BeamModuleAssembler::emit_if_end() {
    emit_error(EXC_IF_CLAUSE);
}
void BeamModuleAssembler::emit_badrecord(const ArgSource &Src) {
    mov_arg(x86::qword_ptr(c_p, offsetof(Process, fvalue)), Src);
    emit_error(EXC_BADRECORD);
}
void BeamModuleAssembler::emit_catch(const ArgYRegister &CatchTag,
                                     const ArgLabel &Handler) {
    a.inc(x86::qword_ptr(c_p, offsetof(Process, catches)));
    Label patch_addr = a.new_label();
    a.bind(patch_addr);
    a.mov(RETd, imm(INT_MAX));
    mov_arg(CatchTag, RET);
    catches.push_back({{patch_addr, 0x1, 0}, resolve_beam_label(Handler)});
}
void BeamGlobalAssembler::emit_catch_end_shared() {
    Label not_throw = a.new_label(), not_error = a.new_label(),
          after_gc = a.new_label();
    emit_enter_frame();
    a.mov(ARG2, getXRef(1));
    a.cmp(getXRef(3), imm(am_throw));
    a.short_().jne(not_throw);
    a.mov(getXRef(0), ARG2);
    emit_leave_frame();
    a.ret();
    a.bind(not_throw);
    {
        a.cmp(getXRef(3), imm(am_error));
        a.jne(not_error);
        emit_enter_runtime<Update::eHeapAlloc>();
        a.mov(ARG1, c_p);
        a.mov(ARG3, getXRef(2));
        runtime_call<Eterm (*)(Process *, Eterm, Eterm), add_stacktrace>();
        emit_leave_runtime<Update::eHeapAlloc>();
        a.mov(ARG2, RET);
    }
    a.bind(not_error);
    {
        const int32_t bytes_needed = (3 + S_RESERVED) * sizeof(Eterm);
        a.lea(ARG3, x86::qword_ptr(HTOP, bytes_needed));
        a.cmp(ARG3, E);
        a.short_().jbe(after_gc);
        a.mov(getXRef(0), ARG2);
        mov_imm(ARG4, 1);
        aligned_call(labels[garbage_collect]);
        a.mov(ARG2, getXRef(0));
        a.bind(after_gc);
        a.mov(x86::qword_ptr(HTOP), imm(make_arityval(2)));
        a.mov(x86::qword_ptr(HTOP, sizeof(Eterm) * 1), imm(am_EXIT));
        a.mov(x86::qword_ptr(HTOP, sizeof(Eterm) * 2), ARG2);
        a.lea(RET, x86::qword_ptr(HTOP, TAG_PRIMARY_BOXED));
        a.add(HTOP, imm(3 * sizeof(Eterm)));
        a.mov(getXRef(0), RET);
    }
    emit_leave_frame();
    a.ret();
}
void BeamModuleAssembler::emit_catch_end(const ArgYRegister &CatchTag) {
    Label next = a.new_label();
    emit_try_end(CatchTag);
    a.cmp(getXRef(0), imm(THE_NON_VALUE));
    a.short_().jne(next);
    fragment_call(ga->get_catch_end_shared());
    a.bind(next);
}
void BeamModuleAssembler::emit_try_end(const ArgYRegister &CatchTag) {
    a.dec(x86::qword_ptr(c_p, offsetof(Process, catches)));
    mov_arg(CatchTag, NIL);
}
void BeamModuleAssembler::emit_try_end_deallocate(const ArgWord &Deallocate) {
    a.dec(x86::qword_ptr(c_p, offsetof(Process, catches)));
    emit_deallocate(Deallocate);
}
void BeamModuleAssembler::emit_try_case(const ArgYRegister &CatchTag) {
    a.dec(x86::qword_ptr(c_p, offsetof(Process, catches)));
    a.mov(RET, getXRef(3));
    a.mov(getXRef(0), RET);
#ifdef DEBUG
    Label fvalue_ok = a.new_label(), assertion_failed = a.new_label();
    comment("Start of assertion code");
    a.cmp(x86::qword_ptr(c_p, offsetof(Process, fvalue)), NIL);
    a.short_().je(fvalue_ok);
    a.bind(assertion_failed);
    comment("Assertion c_p->fvalue == NIL && c_p->ftrace == NIL failed");
    a.ud2();
    a.bind(fvalue_ok);
    a.cmp(x86::qword_ptr(c_p, offsetof(Process, ftrace)), NIL);
    a.short_().jne(assertion_failed);
#endif
}
void BeamModuleAssembler::emit_try_case_end(const ArgSource &Src) {
    mov_arg(x86::qword_ptr(c_p, offsetof(Process, fvalue)), Src);
    emit_error(EXC_TRY_CLAUSE);
}
void BeamGlobalAssembler::emit_raise_shared() {
    a.mov(x86::qword_ptr(c_p, offsetof(Process, fvalue)), ARG3);
    a.mov(x86::qword_ptr(c_p, offsetof(Process, ftrace)), ARG2);
    emit_enter_runtime();
    a.mov(ARG1, c_p);
    runtime_call<void (*)(Process *, Eterm), erts_sanitize_freason>();
    emit_leave_runtime();
    mov_imm(ARG4, 0);
    a.jmp(labels[raise_exception]);
}
void BeamModuleAssembler::emit_raise(const ArgSource &Trace,
                                     const ArgSource &Value) {
    mov_arg(ARG3, Value);
    mov_arg(ARG2, Trace);
    fragment_call(resolve_fragment(ga->get_raise_shared()));
    last_error_offset = a.offset();
}
void BeamModuleAssembler::emit_build_stacktrace() {
    mov_arg(ARG2, ArgXRegister(0));
    emit_enter_runtime<Update::eHeapAlloc>();
    a.mov(ARG1, c_p);
    runtime_call<Eterm (*)(Process *, Eterm), build_stacktrace>();
    emit_leave_runtime<Update::eHeapAlloc>();
    a.mov(getXRef(0), RET);
}
void BeamModuleAssembler::emit_raw_raise() {
    Label next = a.new_label();
    mov_arg(ARG1, ArgXRegister(2));
    mov_arg(ARG2, ArgXRegister(0));
    mov_arg(ARG3, ArgXRegister(1));
    emit_enter_runtime();
    a.mov(ARG4, c_p);
    runtime_call<int (*)(Eterm, Eterm, Eterm, Process *), raw_raise>();
    emit_leave_runtime();
    a.test(RET, RET);
    a.short_().jne(next);
    emit_raise_exception();
    a.bind(next);
    a.mov(getXRef(0), imm(am_badarg));
}
#define TEST_YIELD_RETURN_OFFSET                                               \
    (BEAM_ASM_FUNC_PROLOGUE_SIZE + 16u +                                       \
     (erts_frame_layout == ERTS_FRAME_LAYOUT_FP_RA ? 4u : 0u) +                \
     (erts_alcu_enable_code_atags ? 8u : 0u))
void BeamGlobalAssembler::emit_i_test_yield_shared() {
    int mfa_offset = -TEST_YIELD_RETURN_OFFSET - (int)sizeof(ErtsCodeMFA);
    a.lea(ARG2, x86::qword_ptr(ARG3, mfa_offset));
    a.mov(x86::qword_ptr(c_p, offsetof(Process, current)), ARG2);
    a.movzx(ARG2d, x86::byte_ptr(ARG2, offsetof(ErtsCodeMFA, arity)));
    a.mov(x86::byte_ptr(c_p, offsetof(Process, arity)), ARG2.r8());
    a.jmp(labels[context_switch_simplified]);
}
void BeamModuleAssembler::emit_i_test_yield() {
    ASSERT((a.offset() - code.label_offset_from_base(current_label)) ==
           BEAM_ASM_FUNC_PROLOGUE_SIZE);
    emit_enter_frame();
    a.lea(ARG3, x86::qword_ptr(current_label, TEST_YIELD_RETURN_OFFSET));
    if (erts_alcu_enable_code_atags) {
        a.mov(x86::qword_ptr(c_p, offsetof(Process, i)), ARG3);
    }
    a.dec(FCALLS);
    a.long_().jle(resolve_fragment(ga->get_i_test_yield_shared()));
    a.align(AlignMode::kCode, 4);
    ASSERT((a.offset() - code.label_offset_from_base(current_label)) ==
           TEST_YIELD_RETURN_OFFSET);
#if defined(JIT_HARD_DEBUG) && defined(ERLANG_FRAME_POINTERS)
    a.mov(ARG1, c_p);
    a.mov(ARG2, x86::rbp);
    a.mov(ARG3, x86::rsp);
    emit_enter_runtime<Update::eStack>();
    runtime_call<void (*)(Process *, Eterm *, Eterm *), erts_validate_stack>();
    emit_leave_runtime<Update::eStack>();
#endif
}
void BeamModuleAssembler::emit_i_yield() {
    a.mov(getXRef(0), imm(am_true));
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
#if defined(ERTS_CCONV_DEBUG)
static Uint64 ERTS_CCONV_JIT i_perf_counter(void) {
#    ifdef WIN32
    return erts_sys_time_data__.r.o.sys_hrtime();
#    else
    return erts_sys_time_data__.r.o.perf_counter();
#    endif
}
#endif
void BeamModuleAssembler::emit_i_perf_counter() {
    Label next = a.new_label(), small = a.new_label();
    emit_enter_runtime();
#if defined(ERTS_CCONV_DEBUG)
    runtime_call<Uint64(ERTS_CCONV_JIT *)(void), i_perf_counter>();
#else
#    ifdef WIN32
    mov_imm(RET, erts_sys_time_data__.r.o.sys_hrtime);
#    else
    mov_imm(RET, erts_sys_time_data__.r.o.perf_counter);
#    endif
    dynamic_runtime_call<0>(RET);
#endif
    emit_leave_runtime();
    a.mov(ARG1, RET);
    a.sar(ARG1, imm(SMALL_BITS - 1));
    a.add(ARG1, 1);
    a.cmp(ARG1, 1);
    a.jbe(small);
    {
        a.mov(TMP_MEM1q, RET);
        emit_gc_test(ArgWord(0),
                     ArgWord(ERTS_MAX_UINT64_HEAP_SIZE),
                     ArgWord(0));
        a.mov(ARG1, TMP_MEM1q);
        a.mov(x86::qword_ptr(HTOP, sizeof(Eterm) * 0),
              imm(make_pos_bignum_header(1)));
        a.mov(x86::qword_ptr(HTOP, sizeof(Eterm) * 1), ARG1);
        a.lea(RET, x86::qword_ptr(HTOP, TAG_PRIMARY_BOXED));
        a.add(HTOP, imm(sizeof(Eterm) * 2));
        a.short_().jmp(next);
    }
    a.bind(small);
    {
        a.shl(RET, imm(_TAG_IMMED1_SIZE));
        a.or_(RET, imm(_TAG_IMMED1_SMALL));
    }
    a.bind(next);
    a.mov(getXRef(0), RET);
}
void BeamModuleAssembler::emit_coverage(void *coverage, Uint index, Uint size) {
    Uint address = Uint(coverage) + index * size;
    comment("coverage index = %d", index);
    mov_imm(RET, address);
    if (size == sizeof(Uint)) {
        a.lock().inc(x86::qword_ptr(RET));
    } else if (size == sizeof(byte)) {
        if ((address & 0xff) != 0) {
            a.mov(x86::byte_ptr(RET), RETb);
        } else {
            a.mov(x86::byte_ptr(RET), imm(1));
        }
    } else {
        ASSERT(0);
    }
}
void BeamModuleAssembler::emit_i_debug_line(const ArgWord &Loc,
                                            const ArgWord &Index,
                                            const ArgWord &Live) {
    emit_validate(Live);
    ASSERT(Live.get() <= MAX_ARG);
    a.mov(TMP1, imm(Live.get()));
    a.align(AlignMode::kCode, 8);
}