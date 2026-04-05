#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <functional>
#include <algorithm>
#include <cmath>
#include <asmjit/x86.h>
extern "C"
{
#ifdef HAVE_CONFIG_H
#    include "config.h"
#endif
#include "sys.h"
#include "erl_vm.h"
#include "global.h"
#include "beam_catches.h"
#include "big.h"
#include "beam_asm.h"
}
#include "beam_jit_common.hpp"
#undef min
#undef max
using namespace asmjit;
#if defined(WIN32)
#    define ERTS_JIT_ABI_WIN32
#elif !defined(ERTS_JIT_ABI_WIN32)
#    undef ERTS_JIT_ABI_SYSV
#    define ERTS_JIT_ABI_SYSV
#endif
#if defined(ERTS_JIT_ABI_WIN32) && !defined(WIN32)
#    if defined(__GNUC__) || defined(__clang__)
#        define ERTS_CCONV_ERTS
#        define ERTS_CCONV_JIT __attribute__((ms_abi))
#        define ERTS_CCONV_DEBUG
#    else
#        error "Unsupported JIT debug configuration"
#    endif
#else
#    define ERTS_CCONV_ERTS
#    define ERTS_CCONV_JIT
#endif
struct BeamAssembler : public BeamAssemblerCommon {
    BeamAssembler() : BeamAssemblerCommon(a) {
        Error err = code.attach(&a);
        ERTS_ASSERT(err == Error::kOk && "Failed to attach codeHolder");
        lateInit();
    }
    BeamAssembler(const std::string &log) : BeamAssembler() {
        if (erts_jit_asm_dump) {
            setLogger(log + ".asm");
        }
    }
protected:
    x86::Assembler a;
    const x86::Gp registers = x86::rbx;
#ifdef NATIVE_ERLANG_STACK
    const x86::Gp E = x86::rsp;
#    ifdef ERLANG_FRAME_POINTERS
    const x86::Gp frame_pointer = x86::rbp;
#    endif
    const x86::Gp E_saved = x86::rbp;
#else
    const x86::Gp E = x86::rbp;
#endif
    const x86::Gp c_p = x86::r13;
    const x86::Gp FCALLS = x86::r14d;
    const x86::Gp HTOP = x86::r15;
    const x86::Gp active_code_ix = x86::r12;
#ifdef ERTS_MSACC_EXTENDED_STATES
    const x86::Mem erts_msacc_cache = getSchedulerRegRef(
            offsetof(ErtsSchedulerRegisters, aux_regs.d.erts_msacc_cache));
#endif
#if defined(ERTS_JIT_ABI_WIN32)
    const x86::Gp ARG1 = x86::rcx;
    const x86::Gp ARG2 = x86::rdx;
    const x86::Gp ARG3 = x86::r8;
    const x86::Gp ARG4 = x86::r9;
    const x86::Gp ARG5 = x86::r10;
    const x86::Gp ARG6 = x86::r11;
    const x86::Gp TMP1 = x86::rdi;
    const x86::Gp TMP2 = x86::rsi;
    const x86::Gp ARG1d = x86::ecx;
    const x86::Gp ARG2d = x86::edx;
    const x86::Gp ARG3d = x86::r8d;
    const x86::Gp ARG4d = x86::r9d;
    const x86::Gp ARG5d = x86::r10d;
    const x86::Gp ARG6d = x86::r11d;
#elif defined(ERTS_JIT_ABI_SYSV)
    const x86::Gp ARG1 = x86::rdi;
    const x86::Gp ARG2 = x86::rsi;
    const x86::Gp ARG3 = x86::rdx;
    const x86::Gp ARG4 = x86::rcx;
    const x86::Gp ARG5 = x86::r8;
    const x86::Gp ARG6 = x86::r9;
    const x86::Gp TMP1 = x86::r10;
    const x86::Gp TMP2 = x86::r11;
    const x86::Gp ARG1d = x86::edi;
    const x86::Gp ARG2d = x86::esi;
    const x86::Gp ARG3d = x86::edx;
    const x86::Gp ARG4d = x86::ecx;
    const x86::Gp ARG5d = x86::r8d;
    const x86::Gp ARG6d = x86::r9d;
#endif
    const x86::Gp RET = x86::rax;
    const x86::Gp RETd = x86::eax;
    const x86::Gp RETb = x86::al;
    const x86::Mem TMP_MEM1q = getSchedulerRegRef(
            offsetof(ErtsSchedulerRegisters, aux_regs.d.TMP_MEM[0]));
    const x86::Mem TMP_MEM2q = getSchedulerRegRef(
            offsetof(ErtsSchedulerRegisters, aux_regs.d.TMP_MEM[1]));
    const x86::Mem TMP_MEM3q = getSchedulerRegRef(
            offsetof(ErtsSchedulerRegisters, aux_regs.d.TMP_MEM[2]));
    const x86::Mem TMP_MEM4q = getSchedulerRegRef(
            offsetof(ErtsSchedulerRegisters, aux_regs.d.TMP_MEM[3]));
    const x86::Mem TMP_MEM5q = getSchedulerRegRef(
            offsetof(ErtsSchedulerRegisters, aux_regs.d.TMP_MEM[4]));
    const x86::Mem TMP_MEM1d = getSchedulerRegRef(
            offsetof(ErtsSchedulerRegisters, aux_regs.d.TMP_MEM[0]),
            sizeof(Uint32));
    const x86::Mem TMP_MEM2d = getSchedulerRegRef(
            offsetof(ErtsSchedulerRegisters, aux_regs.d.TMP_MEM[1]),
            sizeof(Uint32));
    const x86::Mem TMP_MEM3d = getSchedulerRegRef(
            offsetof(ErtsSchedulerRegisters, aux_regs.d.TMP_MEM[2]),
            sizeof(Uint32));
    const x86::Mem TMP_MEM4d = getSchedulerRegRef(
            offsetof(ErtsSchedulerRegisters, aux_regs.d.TMP_MEM[3]),
            sizeof(Uint32));
    const x86::Mem TMP_MEM5d = getSchedulerRegRef(
            offsetof(ErtsSchedulerRegisters, aux_regs.d.TMP_MEM[4]),
            sizeof(Uint32));
    enum Distance { dShort, dLong };
    constexpr x86::Mem getRuntimeStackRef() const {
        int base = offsetof(ErtsSchedulerRegisters, aux_regs.d.runtime_stack);
        return getSchedulerRegRef(base);
    }
#if !defined(NATIVE_ERLANG_STACK)
#    ifdef JIT_HARD_DEBUG
    constexpr x86::Mem getInitialSPRef() const {
        int base = offsetof(ErtsSchedulerRegisters, initial_sp);
        return getSchedulerRegRef(base);
    }
#    endif
    constexpr x86::Mem getCPRef() const {
        return x86::qword_ptr(E);
    }
#endif
    constexpr x86::Mem getSchedulerRegRef(int offset,
                                          size_t size = sizeof(UWord)) const {
        const int x_reg_offset =
                offsetof(ErtsSchedulerRegisters, x_reg_array.d);
        ERTS_CT_ASSERT(x_reg_offset <= 128);
        return x86::Mem(registers, offset - x_reg_offset, size);
    }
    constexpr x86::Mem getFRef(int index, size_t size = sizeof(UWord)) const {
        int base = offsetof(ErtsSchedulerRegisters, f_reg_array.d);
        int offset = index * sizeof(FloatDef);
        ASSERT(index >= 0 && index <= 1023);
        return getSchedulerRegRef(base + offset, size);
    }
    constexpr x86::Mem getXRef(int index, size_t size = sizeof(UWord)) const {
        int base = offsetof(ErtsSchedulerRegisters, x_reg_array.d);
        int offset = index * sizeof(Eterm);
        ASSERT(index >= 0 && index < ERTS_X_REGS_ALLOCATED);
        return getSchedulerRegRef(base + offset, size);
    }
    constexpr x86::Mem getYRef(int index, size_t size = sizeof(UWord)) const {
        ASSERT(index >= 0 && index <= 1023);
#ifdef NATIVE_ERLANG_STACK
        return x86::Mem(E, index * sizeof(Eterm), size);
#else
        return x86::Mem(E, (index + CP_SIZE) * sizeof(Eterm), size);
#endif
    }
    constexpr x86::Mem getCARRef(x86::Gp Src,
                                 size_t size = sizeof(UWord)) const {
        return x86::Mem(Src, -TAG_PRIMARY_LIST, size);
    }
    constexpr x86::Mem getCDRRef(x86::Gp Src,
                                 size_t size = sizeof(UWord)) const {
        return x86::Mem(Src, -TAG_PRIMARY_LIST + sizeof(Eterm), size);
    }
    void align_erlang_cp() {
        ERTS_CT_ASSERT(_CPMASK == 3);
        a.align(AlignMode::kCode, 4);
        ASSERT(is_CP(a.offset()));
    }
    void load_x_reg_array(x86::Gp reg) {
        a.mov(reg, registers);
    }
    void load_erl_bits_state(x86::Gp reg) {
        int offset =
                offsetof(ErtsSchedulerRegisters, aux_regs.d.erl_bits_state);
        a.lea(reg, getSchedulerRegRef(offset));
    }
    void emit_assert_redzone_unused() {
#ifdef JIT_HARD_DEBUG
        const int REDZONE_BYTES = S_REDZONE * sizeof(Eterm);
        Label ok = a.new_label(), crash = a.new_label();
        a.sub(E, imm(REDZONE_BYTES));
        a.cmp(HTOP, E);
        a.short_().ja(crash);
        a.cmp(E, x86::qword_ptr(c_p, offsetof(Process, hend)));
        a.short_().jle(ok);
        a.bind(crash);
        comment("Redzone touched");
        a.ud2();
        a.bind(ok);
        a.add(E, imm(REDZONE_BYTES));
#endif
    }
    template<typename Any>
    void erlang_call(Any Target, const x86::Gp &spill) {
#ifdef NATIVE_ERLANG_STACK
        emit_assert_redzone_unused();
        aligned_call(Target);
#else
        Label next = a.new_label();
        a.lea(spill, x86::qword_ptr(next));
        a.mov(getCPRef(), spill);
        a.jmp(Target);
        align_erlang_cp();
        a.bind(next);
#endif
    }
    template<typename Any>
    void fragment_call(Any Target) {
        emit_assert_redzone_unused();
#if defined(JIT_HARD_DEBUG) && !defined(NATIVE_ERLANG_STACK)
        Label next = a.new_label();
        a.cmp(x86::rsp, getInitialSPRef());
        a.short_().je(next);
        comment("The stack has grown");
        a.ud2();
        a.bind(next);
#endif
        aligned_call(Target);
    }
    void safe_fragment_call(void (*Target)()) {
        emit_assert_redzone_unused();
        a.call(imm(Target));
    }
    template<typename FuncPtr>
    void aligned_call(FuncPtr(*target)) {
        aligned_call(imm(target), 6);
    }
    void aligned_call(Label target) {
        aligned_call(target, 5);
    }
    template<typename OperandType>
    void aligned_call(OperandType target) {
        size_t call_offset, call_size;
        call_offset = a.offset();
        a.call(target);
        call_size = a.offset() - call_offset;
        a.set_offset(call_offset);
        aligned_call(target, call_size);
    }
    template<typename OperandType>
    void aligned_call(OperandType target, size_t size) {
        ssize_t next_address = (a.offset() + size);
        ERTS_CT_ASSERT(_CPMASK == 3);
        if (next_address % 4) {
            ssize_t nop_count = 4 - next_address % 4;
            a.embed(nops[nop_count - 1], nop_count);
        }
#ifdef JIT_HARD_DEBUG
#endif
        a.call(target);
        ASSERT(is_CP(a.offset()));
    }
    static const uint8_t *nops[3];
    static const uint8_t nop1[1];
    static const uint8_t nop2[2];
    static const uint8_t nop3[3];
    template<typename T>
    struct function_traits;
    template<typename Range, typename... Domain>
    struct function_traits<Range(ERTS_CCONV_ERTS *)(Domain...)> {
        static constexpr bool Emulator = true;
        static constexpr bool Jit = std::is_same_v<void(ERTS_CCONV_ERTS *)(),
                                                   void(ERTS_CCONV_JIT *)()>;
        static constexpr size_t Arity = sizeof...(Domain);
#ifdef ERTS_CCONV_DEBUG
        template<Range(ERTS_CCONV_ERTS *Func)(Domain...)>
        struct trampoline {
            using type = Range(ERTS_CCONV_JIT *)(Domain...);
            static Range ERTS_CCONV_JIT marshal(Domain... args) {
                ERTS_CT_ASSERT(Emulator != Jit);
                return Func(args...);
            }
        };
#endif
    };
#ifdef ERTS_CCONV_DEBUG
    template<typename Range, typename... Domain>
    struct function_traits<Range(ERTS_CCONV_JIT *)(Domain...)> {
        static constexpr bool Emulator = false;
        static constexpr bool Jit = true;
        static constexpr size_t Arity = sizeof...(Domain);
    };
#endif
    template<typename T,
             T Func,
             std::enable_if_t<!function_traits<T>::Jit, bool> = true>
    void runtime_call() {
        using trampoline =
                typename function_traits<T>::template trampoline<Func>;
        runtime_call<typename trampoline::type, trampoline::marshal>();
    }
    template<typename T,
             T Func,
             std::enable_if_t<function_traits<T>::Jit, bool> = true>
    void runtime_call() {
        emit_assert_runtime_stack();
#if defined(ERTS_JIT_ABI_WIN32)
        unsigned pushed = 4;
        switch (function_traits<T>::Arity) {
        case 6:
        case 5:
            a.push(ARG6);
            a.push(ARG5);
            pushed += 2;
            break;
        }
        a.sub(x86::rsp, imm(4 * sizeof(UWord)));
#endif
        a.call(imm(Func));
#if defined(ERTS_CCONV_DEBUG) && defined(DEBUG)
        a.xor_(ARG1d, ARG1d);
        a.mov(ARG2d, ARG1d);
        a.mov(ARG3d, ARG1d);
        a.mov(ARG4d, ARG1d);
        a.mov(ARG5d, ARG1d);
        a.mov(ARG6d, ARG1d);
        a.mov(TMP1, ARG1);
        a.mov(TMP2, ARG1);
#endif
#if defined(ERTS_JIT_ABI_WIN32)
        a.add(x86::rsp, imm(pushed * sizeof(UWord)));
#endif
    }
    template<int Arity>
    void dynamic_runtime_call(x86::Gp func) {
        emit_assert_runtime_stack();
        ERTS_CT_ASSERT(Arity <= 4);
#ifdef ERTS_JIT_ABI_WIN32
        a.sub(x86::rsp, imm(4 * sizeof(UWord)));
#endif
        a.call(func);
#ifdef ERTS_JIT_ABI_WIN32
        a.add(x86::rsp, imm(4 * sizeof(UWord)));
#endif
    }
    template<typename T>
    void abs_jmp(T(*addr)) {
        a.jmp(imm(addr));
    }
    template<typename T>
    void pic_jmp(T(*addr)) {
        a.mov(ARG6, imm(addr));
        a.jmp(ARG6);
    }
    constexpr x86::Mem getArgRef(const ArgRegister &arg,
                                 size_t size = sizeof(UWord)) const {
        if (arg.isXRegister()) {
            return getXRef(arg.as<ArgXRegister>().get(), size);
        } else if (arg.isYRegister()) {
            return getYRef(arg.as<ArgYRegister>().get(), size);
        }
        return getFRef(arg.as<ArgFRegister>().get(), size);
    }
    x86::Mem emit_setup_dispatchable_call(const x86::Gp &Src) {
        return emit_setup_dispatchable_call(Src, active_code_ix);
    }
    x86::Mem emit_setup_dispatchable_call(const x86::Gp &Src,
                                          const x86::Gp &CodeIndex) {
        if (RET != Src) {
            a.mov(RET, Src);
        }
        ERTS_CT_ASSERT(offsetof(ErlFunEntry, dispatch) == 0);
        ERTS_CT_ASSERT(offsetof(Export, dispatch) == 0);
        return x86::qword_ptr(RET,
                              CodeIndex,
                              3,
                              offsetof(ErtsDispatchable, addresses));
    }
    void emit_assert_runtime_stack() {
#ifdef JIT_HARD_DEBUG
        Label crash = a.new_label(), next = a.new_label();
#    ifdef NATIVE_ERLANG_STACK
        int end_offs, start_offs;
        end_offs = offsetof(ErtsSchedulerRegisters, runtime_stack_end);
        start_offs = offsetof(ErtsSchedulerRegisters, runtime_stack_start);
        a.cmp(E, getSchedulerRegRef(end_offs));
        a.short_().jbe(crash);
        a.cmp(E, getSchedulerRegRef(start_offs));
        a.short_().ja(crash);
#    endif
        a.test(x86::rsp, (16 - 1));
        a.short_().je(next);
        a.bind(crash);
        comment("Runtime stack is corrupt");
        a.ud2();
        a.bind(next);
#endif
    }
    void emit_assert_erlang_stack() {
#ifdef JIT_HARD_DEBUG
        Label crash = a.new_label(), next = a.new_label();
        a.test(E, imm(sizeof(Eterm) - 1));
        a.short_().jne(crash);
        a.cmp(E, x86::qword_ptr(c_p, offsetof(Process, heap)));
        a.short_().jl(crash);
        a.cmp(E, x86::qword_ptr(c_p, offsetof(Process, hend)));
        a.short_().jle(next);
        a.bind(crash);
        comment("Erlang stack is corrupt");
        a.ud2();
        a.bind(next);
#endif
    }
    enum Update : int {
        eStack = (1 << 0),
        eHeap = (1 << 1),
        eReductions = (1 << 2),
        eCodeIndex = (1 << 3),
        eHeapAlloc = Update::eHeap | Update::eStack,
#ifndef DEBUG
        eHeapOnlyAlloc = Update::eHeap,
#else
        eHeapOnlyAlloc = Update::eHeapAlloc
#endif
    };
    void emit_enter_frame() {
#ifdef NATIVE_ERLANG_STACK
        if (ERTS_UNLIKELY(erts_frame_layout == ERTS_FRAME_LAYOUT_FP_RA)) {
#    ifdef ERLANG_FRAME_POINTERS
            a.push(frame_pointer);
            a.mov(frame_pointer, E);
#    endif
        } else {
            ASSERT(erts_frame_layout == ERTS_FRAME_LAYOUT_RA);
        }
#endif
    }
    void emit_leave_frame() {
#ifdef NATIVE_ERLANG_STACK
        if (ERTS_UNLIKELY(erts_frame_layout == ERTS_FRAME_LAYOUT_FP_RA)) {
            a.leave();
        } else {
            ASSERT(erts_frame_layout == ERTS_FRAME_LAYOUT_RA);
        }
#endif
    }
    void emit_unwind_frame() {
        emit_assert_erlang_stack();
        emit_leave_frame();
        a.add(x86::rsp, imm(sizeof(UWord)));
    }
    template<int Spec = 0>
    void emit_enter_runtime() {
        emit_assert_erlang_stack();
        ERTS_CT_ASSERT((Spec & (Update::eReductions | Update::eStack |
                                Update::eHeap)) == Spec);
        if (ERTS_LIKELY(erts_frame_layout == ERTS_FRAME_LAYOUT_RA)) {
            if ((Spec & (Update::eHeap | Update::eStack)) ==
                (Update::eHeap | Update::eStack)) {
                ERTS_CT_ASSERT((offsetof(Process, stop) -
                                offsetof(Process, htop)) == sizeof(Eterm *));
                if (hasCpuFeature(CpuFeatures::X86::kAVX)) {
                    a.vmovq(x86::xmm1, HTOP);
                    a.vpinsrq(x86::xmm0, x86::xmm1, E, 1);
                    a.vmovdqu(x86::xmmword_ptr(c_p, offsetof(Process, htop)),
                              x86::xmm0);
                } else {
                    a.movq(x86::xmm0, HTOP);
                    a.movq(x86::xmm1, E);
                    a.punpcklqdq(x86::xmm0, x86::xmm1);
                    a.movups(x86::xmmword_ptr(c_p, offsetof(Process, htop)),
                             x86::xmm0);
                }
            } else {
                if (Spec & Update::eHeap) {
                    a.mov(x86::qword_ptr(c_p, offsetof(Process, htop)), HTOP);
                } else {
#ifdef DEBUG
                    a.mov(x86::qword_ptr(c_p, offsetof(Process, htop)),
                          active_code_ix);
#endif
                }
                if (Spec & Update::eStack) {
                    a.mov(x86::qword_ptr(c_p, offsetof(Process, stop)), E);
                } else {
#ifdef DEBUG
                    a.mov(x86::qword_ptr(c_p, offsetof(Process, stop)),
                          active_code_ix);
#endif
                }
            }
#ifdef NATIVE_ERLANG_STACK
            if (!(Spec & Update::eStack)) {
                a.mov(E_saved, E);
            }
#endif
        } else {
#ifdef ERLANG_FRAME_POINTERS
            ASSERT(erts_frame_layout == ERTS_FRAME_LAYOUT_FP_RA);
            if (Spec & Update::eStack) {
                ERTS_CT_ASSERT((offsetof(Process, frame_pointer) -
                                offsetof(Process, stop)) == sizeof(Eterm *));
                if (hasCpuFeature(CpuFeatures::X86::kAVX)) {
                    a.vmovq(x86::xmm1, E);
                    a.vpinsrq(x86::xmm0, x86::xmm1, frame_pointer, 1);
                    a.vmovdqu(x86::xmmword_ptr(c_p, offsetof(Process, stop)),
                              x86::xmm0);
                } else {
                    a.movq(x86::xmm0, E);
                    a.movq(x86::xmm1, frame_pointer);
                    a.punpcklqdq(x86::xmm0, x86::xmm1);
                    a.movups(x86::xmmword_ptr(c_p, offsetof(Process, stop)),
                             x86::xmm0);
                }
            } else {
                a.mov(x86::qword_ptr(c_p, offsetof(Process, stop)), E);
            }
            if (Spec & Update::eHeap) {
                a.mov(x86::qword_ptr(c_p, offsetof(Process, htop)), HTOP);
            }
#endif
        }
        if (Spec & Update::eReductions) {
            a.mov(x86::dword_ptr(c_p, offsetof(Process, fcalls)), FCALLS);
        }
#ifdef NATIVE_ERLANG_STACK
        a.lea(E, getRuntimeStackRef());
#else
        a.mov(getRuntimeStackRef(), x86::rsp);
        a.sub(x86::rsp, imm(15));
        a.and_(x86::rsp, imm(-16));
#endif
#if !defined(__AVX__)
        if (hasCpuFeature(CpuFeatures::X86::kAVX)) {
            a.vzeroupper();
        }
#endif
    }
    template<int Spec = 0>
    void emit_leave_runtime() {
        emit_assert_runtime_stack();
        ERTS_CT_ASSERT((Spec & (Update::eReductions | Update::eStack |
                                Update::eHeap | Update::eCodeIndex)) == Spec);
        if (ERTS_LIKELY(erts_frame_layout == ERTS_FRAME_LAYOUT_RA)) {
            if (Spec & Update::eStack) {
                a.mov(E, x86::qword_ptr(c_p, offsetof(Process, stop)));
            } else {
#ifdef NATIVE_ERLANG_STACK
                a.mov(E, E_saved);
#endif
            }
        } else {
#ifdef ERLANG_FRAME_POINTERS
            ASSERT(erts_frame_layout == ERTS_FRAME_LAYOUT_FP_RA);
            a.mov(E, x86::qword_ptr(c_p, offsetof(Process, stop)));
            if (Spec & Update::eStack) {
                a.mov(frame_pointer,
                      x86::qword_ptr(c_p, offsetof(Process, frame_pointer)));
            }
#endif
        }
        if (Spec & Update::eHeap) {
            a.mov(HTOP, x86::qword_ptr(c_p, offsetof(Process, htop)));
        }
        if (Spec & Update::eReductions) {
            a.mov(FCALLS, x86::dword_ptr(c_p, offsetof(Process, fcalls)));
        }
        if (Spec & Update::eCodeIndex) {
            a.mov(ARG1, imm(&the_active_code_index));
            a.mov(ARG1d, x86::dword_ptr(ARG1));
            a.cmp(active_code_ix, imm(ERTS_SAVE_CALLS_CODE_IX));
            a.cmovne(active_code_ix, ARG1);
        }
#if !defined(NATIVE_ERLANG_STACK)
        a.mov(x86::rsp, getRuntimeStackRef());
#endif
    }
    void emit_test(x86::Gp Src, byte mask) {
        if (Src == x86::rax || Src == x86::rdi || Src == x86::rsi ||
            Src == x86::rcx || Src == x86::rdx) {
            a.test(Src.r8(), imm(mask));
        } else {
            a.test(Src.r32(), imm(mask));
        }
    }
    void emit_test_cons(x86::Gp Src) {
        emit_test(Src, _TAG_PRIMARY_MASK - TAG_PRIMARY_LIST);
    }
    void emit_is_cons(Label Fail, x86::Gp Src, Distance dist = dLong) {
        emit_test_cons(Src);
        if (dist == dShort) {
            a.short_().jne(Fail);
        } else {
            a.jne(Fail);
        }
    }
    void emit_is_not_cons(Label Fail, x86::Gp Src, Distance dist = dLong) {
        emit_test_cons(Src);
        if (dist == dShort) {
            a.short_().je(Fail);
        } else {
            a.je(Fail);
        }
    }
    void emit_test_boxed(x86::Gp Src) {
        emit_test(Src, _TAG_PRIMARY_MASK - TAG_PRIMARY_BOXED);
    }
    void emit_is_boxed(Label Fail, x86::Gp Src, Distance dist = dLong) {
        emit_test_boxed(Src);
        if (dist == dShort) {
            a.short_().jne(Fail);
        } else {
            a.jne(Fail);
        }
    }
    void emit_is_not_boxed(Label Fail, x86::Gp Src, Distance dist = dLong) {
        emit_test_boxed(Src);
        if (dist == dShort) {
            a.short_().je(Fail);
        } else {
            a.je(Fail);
        }
    }
    x86::Gp emit_ptr_val(x86::Gp Dst, x86::Gp Src) {
#if !defined(TAG_LITERAL_PTR)
        return Src;
#else
        if (Dst != Src) {
            a.mov(Dst, Src);
        }
        a.and_(Dst, imm(~TAG_LITERAL_PTR));
        return Dst;
#endif
    }
    constexpr x86::Mem emit_boxed_val(x86::Gp Src,
                                      int32_t bytes = 0,
                                      size_t size = sizeof(UWord)) const {
        ASSERT(bytes % sizeof(Eterm) == 0);
        return x86::Mem(Src, bytes - TAG_PRIMARY_BOXED, size);
    }
    void emit_test_the_non_value(x86::Gp Reg) {
        if (THE_NON_VALUE == 0) {
            a.test(Reg.r32(), Reg.r32());
        } else {
            a.cmp(Reg, imm(THE_NON_VALUE));
        }
    }
    template<typename T>
    void mov_imm(x86::Gp to, T value) {
        static_assert(std::is_integral<T>::value || std::is_pointer<T>::value);
        if (value) {
            if (Support::is_uint_n<32>((Uint)value)) {
                a.mov(to.r32(), imm(value));
            } else {
                a.mov(to, imm(value));
            }
        } else {
            a.xor_(to.r32(), to.r32());
        }
    }
    void mov_imm(x86::Gp to, std::nullptr_t value) {
        (void)value;
        mov_imm(to, 0);
    }
    template<typename Dst, typename Src>
    void vmovups(Dst dst, Src src) {
        if (hasCpuFeature(CpuFeatures::X86::kAVX)) {
            a.vmovups(dst, src);
        } else {
            a.movups(dst, src);
        }
    }
    template<typename Dst, typename Src>
    void vmovsd(Dst dst, Src src) {
        if (hasCpuFeature(CpuFeatures::X86::kAVX)) {
            a.vmovsd(dst, src);
        } else {
            a.movsd(dst, src);
        }
    }
    template<typename Dst, typename Src>
    void vucomisd(Dst dst, Src src) {
        if (hasCpuFeature(CpuFeatures::X86::kAVX)) {
            a.vucomisd(dst, src);
        } else {
            a.ucomisd(dst, src);
        }
    }
    void emit_copy_words(x86::Mem from,
                         x86::Mem to,
                         Sint32 count,
                         x86::Gp spill) {
        ASSERT(!from.has_index() && !to.has_index());
        ASSERT(count >= 0 && count < (ERTS_SINT32_MAX / (Sint32)sizeof(UWord)));
        ASSERT(from.offset() < ERTS_SINT32_MAX - count * (Sint32)sizeof(UWord));
        ASSERT(to.offset() < ERTS_SINT32_MAX - count * (Sint32)sizeof(UWord));
        from.set_size(0);
        to.set_size(0);
        using vectors = std::initializer_list<std::tuple<x86::Vec,
                                                         Sint32,
                                                         x86::Inst::Id,
                                                         CpuFeatures::X86::Id>>;
        for (const auto &spec : vectors{{x86::zmm0,
                                         8,
                                         x86::Inst::kIdVmovups,
                                         CpuFeatures::X86::kAVX512_VL},
                                        {x86::zmm0,
                                         8,
                                         x86::Inst::kIdVmovups,
                                         CpuFeatures::X86::kAVX512_F},
                                        {x86::ymm0,
                                         4,
                                         x86::Inst::kIdVmovups,
                                         CpuFeatures::X86::kAVX},
                                        {x86::xmm0,
                                         2,
                                         x86::Inst::kIdVmovups,
                                         CpuFeatures::X86::kAVX},
                                        {x86::xmm0,
                                         2,
                                         x86::Inst::kIdMovups,
                                         CpuFeatures::X86::kSSE}}) {
            const auto &[vector_reg, vector_size, vector_inst, feature] = spec;
            if (!hasCpuFeature(feature)) {
                continue;
            }
            if (count <= vector_size * 4) {
                while (count >= vector_size) {
                    a.emit(vector_inst, vector_reg, from);
                    a.emit(vector_inst, to, vector_reg);
                    from.add_offset(sizeof(UWord) * vector_size);
                    to.add_offset(sizeof(UWord) * vector_size);
                    count -= vector_size;
                }
            } else {
                Sint32 loop_iterations, loop_size;
                Label copy_next = a.new_label();
                loop_iterations = count / vector_size;
                loop_size = loop_iterations * vector_size * sizeof(UWord);
                from.add_offset(loop_size);
                to.add_offset(loop_size);
                from.set_index(spill);
                to.set_index(spill);
                mov_imm(spill, -loop_size);
                a.bind(copy_next);
                {
                    a.emit(vector_inst, vector_reg, from);
                    a.emit(vector_inst, to, vector_reg);
                    a.add(spill, imm(vector_size * sizeof(UWord)));
                    a.short_().jne(copy_next);
                }
                from.reset_index();
                to.reset_index();
                count %= vector_size;
            }
        }
        if (count == 1) {
            a.mov(spill, from);
            a.mov(to, spill);
            count -= 1;
        }
        ASSERT(count == 0);
        (void)count;
    }
};
#include "beam_asm_global.hpp"
class BeamModuleAssembler : public BeamAssembler,
                            public BeamModuleAssemblerCommon {
    BeamGlobalAssembler *ga;
    size_t last_error_offset = 0;
    RegisterCache<20, x86::Mem, x86::Gp> reg_cache =
            RegisterCache<20, x86::Mem, x86::Gp>(
                    registers,
                    E,
                    {TMP1, TMP2, ARG3, ARG4, ARG5, ARG6, ARG1, RET});
    x86::Gp find_cache(x86::Mem mem) {
        return reg_cache.find(a.offset(), mem);
    }
    void store_cache(x86::Gp src, x86::Mem mem_dst) {
        reg_cache.consolidate(a.offset());
        a.mov(mem_dst, src);
        reg_cache.put(mem_dst, src);
        reg_cache.update(a.offset());
    }
    void load_cached(x86::Gp dst, x86::Mem mem) {
        x86::Gp cached_reg = find_cache(mem);
        if (cached_reg.is_valid()) {
            if (cached_reg == dst) {
                comment("skipped fetching of BEAM register");
            } else {
                comment("simplified fetching of BEAM register");
                a.mov(dst, cached_reg);
                reg_cache.invalidate(dst);
                reg_cache.update(a.offset());
            }
        } else {
            a.mov(dst, mem);
            reg_cache.invalidate(dst);
            reg_cache.put(mem, dst);
            reg_cache.update(a.offset());
        }
    }
    template<typename L, typename... Any>
    void preserve_cache(L generate, Any... clobber) {
        bool valid = reg_cache.validAt(a.offset());
        generate();
        if (valid) {
            if (sizeof...(clobber) > 0) {
                reg_cache.invalidate(clobber...);
            }
            reg_cache.update(a.offset());
        }
    }
    void trim_preserve_cache(const ArgWord &Words) {
        if (Words.get() > 0) {
            ASSERT(Words.get() <= 1023);
            preserve_cache([&]() {
                auto offset = Words.get() * sizeof(Eterm);
                a.add(E, imm(offset));
                reg_cache.trim_yregs(-offset);
            });
        }
    }
    void mov_preserve_cache(x86::Mem dst, x86::Gp src) {
        preserve_cache(
                [&]() {
                    a.mov(dst, src);
                },
                dst);
    }
    void mov_preserve_cache(x86::Gp dst, x86::Gp src) {
        preserve_cache(
                [&]() {
                    a.mov(dst, src);
                },
                dst);
    }
    void mov_preserve_cache(x86::Gp dst, x86::Mem src) {
        preserve_cache(
                [&]() {
                    a.mov(dst, src);
                },
                dst);
    }
    void cmp_preserve_cache(x86::Gp reg1, x86::Gp reg2) {
        preserve_cache([&]() {
            a.cmp(reg1, reg2);
        });
    }
    void cmp_preserve_cache(x86::Mem mem, x86::Gp reg) {
        preserve_cache([&]() {
            a.cmp(mem, reg);
        });
    }
    x86::Gp alloc_temp_reg() {
        return reg_cache.allocate(a.offset());
    }
    std::unordered_map<void (*)(), Label> _dispatchTable;
public:
    BeamModuleAssembler(BeamGlobalAssembler *ga,
                        Eterm mod,
                        int num_labels,
                        const BeamFile *file = NULL);
    BeamModuleAssembler(BeamGlobalAssembler *ga,
                        Eterm mod,
                        int num_labels,
                        int num_functions,
                        const BeamFile *file = NULL);
    bool emit(unsigned op, const Span<const ArgVal> &args);
    void emit_coverage(void *coverage, Uint index, Uint size);
    void codegen(JitAllocator *allocator,
                 const void **executable_ptr,
                 void **writable_ptr,
                 const BeamCodeHeader *in_hdr,
                 const BeamCodeHeader **out_exec_hdr,
                 BeamCodeHeader **out_rw_hdr);
    void codegen(JitAllocator *allocator,
                 const void **executable_ptr,
                 void **writable_ptr);
    void codegen(char *buff, size_t len);
    void *register_metadata(const BeamCodeHeader *header);
    ErtsCodePtr getCode(unsigned label);
    ErtsCodePtr getLambda(unsigned index);
    void *getCode(Label label) {
        return BeamAssembler::getCode(label);
    }
    byte *getCode(char *labelName) {
        return BeamAssembler::getCode(labelName);
    }
    void embed_vararg_rodata(const Span<const ArgVal> &args,
                             x86::Gp reg,
                             int y_offset);
    unsigned getCodeSize() {
        ASSERT(code.has_base_address());
        return code.code_size();
    }
    void copyCodeHeader(BeamCodeHeader *hdr);
    BeamCodeHeader *getCodeHeader(void);
    const ErtsCodeInfo *getOnLoad(void);
    unsigned patchCatches(char *rw_base);
    void patchLambda(char *rw_base, unsigned index, const ErlFunEntry *fe);
    void patchLiteral(char *rw_base, unsigned index, Eterm lit);
    void patchImport(char *rw_base, unsigned index, const Export *import);
    void patchStrings(char *rw_base, const byte *string);
protected:
    void emit_gc_test(const ArgWord &Stack,
                      const ArgWord &Heap,
                      const ArgWord &Live);
    void emit_gc_test_preserve(const ArgWord &Need,
                               const ArgWord &Live,
                               const ArgSource &Preserve,
                               x86::Gp preserve_reg);
    x86::Mem emit_variable_apply(bool includeI);
    x86::Mem emit_fixed_apply(const ArgWord &arity, bool includeI);
    x86::Gp emit_call_fun(bool skip_box_test = false,
                          bool skip_header_test = false);
    void emit_is_boxed(Label Fail, x86::Gp Src, Distance dist = dLong) {
        preserve_cache([&]() {
            BeamAssembler::emit_is_boxed(Fail, Src, dist);
        });
    }
    void emit_is_boxed(Label Fail,
                       const ArgVal &Arg,
                       x86::Gp Src,
                       Distance dist = dLong) {
        if (always_one_of<BeamTypeId::AlwaysBoxed>(Arg)) {
            comment("skipped box test since argument is always boxed");
            return;
        }
        preserve_cache([&]() {
            BeamAssembler::emit_is_boxed(Fail, Src, dist);
        });
    }
    void emit_is_cons(Label Fail, x86::Gp Src, Distance dist = dLong) {
        preserve_cache([&]() {
            emit_test_cons(Src);
            if (dist == dShort) {
                a.short_().jne(Fail);
            } else {
                a.jne(Fail);
            }
        });
    }
    void emit_is_not_cons(Label Fail, x86::Gp Src, Distance dist = dLong) {
        preserve_cache([&]() {
            emit_test_cons(Src);
            if (dist == dShort) {
                a.short_().je(Fail);
            } else {
                a.je(Fail);
            }
        });
    }
    void emit_get_list(const x86::Gp boxed_ptr,
                       const ArgRegister &Hd,
                       const ArgRegister &Tl);
    void emit_div_rem(const ArgLabel &Fail,
                      const ArgSource &LHS,
                      const ArgSource &RHS,
                      const ErtsCodeMFA *error_mfa,
                      bool need_div = true,
                      bool need_rem = true);
    void emit_setup_guard_bif(const std::vector<ArgVal> &args,
                              const ArgWord &bif);
    void emit_error(int code);
    void emit_bs_get_integer(const ArgRegister &Ctx,
                             const ArgLabel &Fail,
                             const ArgWord &Live,
                             const ArgWord Flags,
                             int bits,
                             const ArgRegister &Dst);
    int emit_bs_get_field_size(const ArgSource &Size,
                               int unit,
                               Label Fail,
                               const x86::Gp &out,
                               unsigned max_size = 0);
    void emit_bs_get_small(const Label &fail,
                           const ArgRegister &Ctx,
                           const ArgWord &Live,
                           const ArgSource &Sz,
                           Uint unit,
                           Uint flags);
    void emit_bs_get_any_int(const Label &fail,
                             const ArgRegister &Ctx,
                             const ArgWord &Live,
                             const ArgSource &Sz,
                             Uint unit,
                             Uint flags);
    void emit_bs_get_binary(const ArgWord heap_need,
                            const ArgRegister &Ctx,
                            const ArgLabel &Fail,
                            const ArgWord &Live,
                            const ArgSource &Size,
                            const ArgWord &Unit,
                            const ArgRegister &Dst);
    void emit_bs_get_utf8(const ArgRegister &Ctx, const ArgLabel &Fail);
    void emit_bs_get_utf16(const ArgRegister &Ctx,
                           const ArgLabel &Fail,
                           const ArgWord &Flags);
    void update_bin_state(x86::Gp bin_offset,
                          x86::Gp current_byte,
                          Sint bit_offset,
                          Sint size,
                          x86::Gp size_reg);
    bool need_mask(const ArgVal Val, Sint size);
    void set_zero(Sint effectiveSize);
    void emit_accumulate(ArgVal src,
                         Sint effectiveSize,
                         x86::Gp bin_data,
                         x86::Gp tmp,
                         x86::Gp value,
                         bool isFirst);
    bool bs_maybe_enter_runtime(bool entered);
    void bs_maybe_leave_runtime(bool entered);
    void emit_construct_utf8_shared();
    void emit_construct_utf8(const ArgVal &Src,
                             Sint bit_offset,
                             bool is_byte_aligned);
    void emit_read_bits(Uint bits,
                        const x86::Gp bin_base,
                        const x86::Gp bin_offset,
                        const x86::Gp bitdata);
    void emit_extract_integer(const x86::Gp bitdata,
                              const x86::Gp tmp,
                              Uint flags,
                              Uint bits,
                              const ArgRegister &Dst);
    void emit_extract_bitstring(const x86::Gp bitdata,
                                Uint bits,
                                const ArgRegister &Dst);
    void emit_read_integer(const x86::Gp bin_base,
                           const x86::Gp bin_position,
                           const x86::Gp tmp,
                           Uint flags,
                           Uint bits,
                           const ArgRegister &Dst);
    UWord bs_get_flags(const ArgVal &val);
    void emit_raise_exception();
    void emit_raise_exception(const ErtsCodeMFA *exp);
    void emit_raise_exception(Label I, const ErtsCodeMFA *exp);
    void emit_raise_exception(x86::Gp I, const ErtsCodeMFA *exp);
    void emit_validate(const ArgWord &arity);
    void emit_bs_skip_bits(const ArgLabel &Fail, const ArgRegister &Ctx);
    void emit_linear_search(x86::Gp val,
                            const ArgVal &Fail,
                            const Span<const ArgVal> &args);
    void emit_float_instr(uint32_t instIdSSE,
                          uint32_t instIdAVX,
                          const ArgFRegister &LHS,
                          const ArgFRegister &RHS,
                          const ArgFRegister &Dst);
    void emit_is_small(Label fail, const ArgSource &Arg, x86::Gp Reg);
    void emit_are_both_small(Label fail,
                             const ArgSource &LHS,
                             x86::Gp A,
                             const ArgSource &RHS,
                             x86::Gp B);
    void emit_validate_unicode(Label next, Label fail, x86::Gp value);
    void emit_bif_is_eq_ne_exact(const ArgSource &LHS,
                                 const ArgSource &RHS,
                                 const ArgRegister &Dst,
                                 Eterm fail_value,
                                 Eterm succ_value);
    void emit_cond_to_bool(uint32_t instId, const ArgRegister &Dst);
    void emit_bif_is_ge_lt(uint32_t instId,
                           const ArgSource &LHS,
                           const ArgSource &RHS,
                           const ArgRegister &Dst);
    void emit_bif_min_max(uint32_t instId,
                          const ArgSource &LHS,
                          const ArgSource &RHS,
                          const ArgRegister &Dst);
    void emit_proc_lc_unrequire(void);
    void emit_proc_lc_require(void);
    void emit_nyi(const char *msg);
    void emit_nyi(void);
    void emit_binsearch_nodes(size_t Left,
                              size_t Right,
                              const ArgVal &Fail,
                              const Span<const ArgVal> &args);
    bool emit_optimized_two_way_select(bool destructive,
                                       const ArgVal &value1,
                                       const ArgVal &value2,
                                       const ArgVal &label);
#ifdef DEBUG
    void emit_tuple_assertion(const ArgSource &Src, x86::Gp tuple_reg);
#endif
    void emit_return_do(bool set_I);
#include "beamasm_protos.h"
    const Label &resolve_beam_label(const ArgLabel &Lbl) const {
        return rawLabels.at(Lbl.get());
    }
    const Label &resolve_fragment(void (*fragment)());
    void flush_last_error();
    void safe_fragment_call(void (*fragment)()) {
        emit_assert_redzone_unused();
        a.call(resolve_fragment(fragment));
    }
    template<typename FuncPtr>
    void aligned_call(FuncPtr(*target)) {
        BeamAssembler::aligned_call(resolve_fragment(target));
    }
    void aligned_call(Label target) {
        BeamAssembler::aligned_call(target);
    }
    void make_move_patch(x86::Gp to,
                         std::vector<struct patch> &patches,
                         size_t offset = 0) {
        const int MOV_IMM64_PAYLOAD_OFFSET = 2;
        Label lbl = a.new_label();
        a.bind(lbl);
        a.long_().mov(to, imm(LLONG_MAX));
        patches.push_back({lbl, MOV_IMM64_PAYLOAD_OFFSET, offset});
    }
    void make_word_patch(std::vector<struct patch> &patches) {
        Label lbl = a.new_label();
        a.bind(lbl);
        a.embed_uint64(LLONG_MAX);
        patches.push_back({lbl, 0, 0});
    }
    template<typename A, typename B>
    void mov_arg(A to, B from) {
        emit_assert_erlang_stack();
        mov_arg(to, from, ARG1);
    }
    template<typename T>
    void cmp_arg(T oper, const ArgVal &val) {
        cmp_arg(oper, val, ARG1);
    }
    void cmp_arg(x86::Mem mem, const ArgVal &val, const x86::Gp &spill) {
        x86::Gp reg = find_cache(mem);
        if (reg.is_valid()) {
            if (val.isImmed() &&
                Support::is_int_n<32>((Sint)val.as<ArgImmed>().get())) {
                comment("simplified compare of BEAM register");
                preserve_cache([&]() {
                    a.cmp(reg, imm(val.as<ArgImmed>().get()));
                });
            } else if (reg != spill) {
                comment("simplified compare of BEAM register");
                mov_arg(spill, val);
                cmp_preserve_cache(reg, spill);
            } else {
                mov_arg(spill, val);
                cmp_preserve_cache(mem, spill);
            }
        } else {
            if (val.isImmed() &&
                Support::is_int_n<32>((Sint)val.as<ArgImmed>().get())) {
                preserve_cache([&]() {
                    a.cmp(mem, imm(val.as<ArgImmed>().get()));
                });
            } else {
                mov_arg(spill, val);
                cmp_preserve_cache(mem, spill);
            }
        }
    }
    void cmp_arg(x86::Gp gp, const ArgVal &val, const x86::Gp &spill) {
        if (val.isImmed() &&
            Support::is_int_n<32>((Sint)val.as<ArgImmed>().get())) {
            preserve_cache([&]() {
                a.cmp(gp, imm(val.as<ArgImmed>().get()));
            });
        } else {
            mov_arg(spill, val);
            cmp_preserve_cache(gp, spill);
        }
    }
    void cmp(x86::Gp gp, int64_t val, const x86::Gp &spill) {
        if (Support::is_int_n<32>(val)) {
            preserve_cache([&]() {
                a.cmp(gp, imm(val));
            });
        } else if (gp.is_gp32()) {
            mov_imm(spill, val);
            preserve_cache([&]() {
                a.cmp(gp, spill.r32());
            });
        } else {
            mov_imm(spill, val);
            cmp_preserve_cache(gp, spill);
        }
    }
    void sub(x86::Gp gp, int64_t val, const x86::Gp &spill) {
        if (Support::is_int_n<32>(val)) {
            preserve_cache(
                    [&]() {
                        a.sub(gp, imm(val));
                    },
                    gp);
        } else {
            preserve_cache(
                    [&]() {
                        mov_imm(spill, val);
                        a.sub(gp, spill);
                    },
                    gp,
                    spill);
        }
    }
    void mov_arg(x86::Gp to, const ArgVal &from, const x86::Gp &spill) {
        if (from.isBytePtr()) {
            make_move_patch(to, strings, from.as<ArgBytePtr>().get());
        } else if (from.isExport()) {
            make_move_patch(to, imports[from.as<ArgExport>().get()].patches);
        } else if (from.isImmed()) {
            preserve_cache(
                    [&]() {
                        mov_imm(to, from.as<ArgImmed>().get());
                    },
                    to);
        } else if (from.isLambda()) {
            preserve_cache(
                    [&]() {
                        make_move_patch(
                                to,
                                lambdas[from.as<ArgLambda>().get()].patches);
                    },
                    to);
        } else if (from.isLiteral()) {
            preserve_cache(
                    [&]() {
                        make_move_patch(
                                to,
                                literals[from.as<ArgLiteral>().get()].patches);
                    },
                    to);
        } else if (from.isRegister()) {
            auto mem = getArgRef(from.as<ArgRegister>());
            load_cached(to, mem);
        } else if (from.isWord()) {
            preserve_cache(
                    [&]() {
                        mov_imm(to, from.as<ArgWord>().get());
                    },
                    to);
        } else {
            ASSERT(!"mov_arg with incompatible type");
        }
#ifdef DEBUG
        a.test(to, to);
#endif
    }
    void mov_arg(x86::Mem to, const ArgVal &from, const x86::Gp &spill) {
        if (from.isImmed()) {
            auto val = from.as<ArgImmed>().get();
            if (Support::is_int_n<32>((Sint)val)) {
                preserve_cache(
                        [&]() {
                            a.mov(to, imm(val));
                        },
                        to);
            } else {
                preserve_cache(
                        [&]() {
                            a.mov(spill, imm(val));
                            a.mov(to, spill);
                        },
                        to,
                        spill);
            }
        } else if (from.isWord()) {
            auto val = from.as<ArgWord>().get();
            if (Support::is_int_n<32>((Sint)val)) {
                preserve_cache(
                        [&]() {
                            a.mov(to, imm(val));
                        },
                        to);
            } else {
                preserve_cache(
                        [&]() {
                            a.mov(spill, imm(val));
                            a.mov(to, spill);
                        },
                        to,
                        spill);
            }
        } else {
            mov_arg(spill, from);
            mov_preserve_cache(to, spill);
        }
    }
    void mov_arg(const ArgRegister &to, x86::Gp from, const x86::Gp &spill) {
        (void)spill;
        auto mem = getArgRef(to);
        store_cache(from, mem);
    }
    void mov_arg(const ArgRegister &to, x86::Mem from, const x86::Gp &spill) {
        a.mov(spill, from);
        a.mov(getArgRef(to), spill);
    }
    void mov_arg(const ArgRegister &to, BeamInstr from, const x86::Gp &spill) {
        preserve_cache(
                [&]() {
                    if (Support::is_int_n<32>((Sint)from)) {
                        a.mov(getArgRef(to), imm(from));
                    } else {
                        a.mov(spill, imm(from));
                        mov_arg(to, spill);
                    }
                },
                getArgRef(to),
                spill);
    }
    void mov_arg(const ArgRegister &to,
                 const ArgVal &from,
                 const x86::Gp &spill) {
        if (!from.isRegister()) {
            mov_arg(getArgRef(to), from);
        } else {
            x86::Gp from_reg = find_cache(getArgRef(from));
            if (from_reg.is_valid()) {
                comment("skipped fetching of BEAM register");
            } else {
                from_reg = spill;
                mov_arg(from_reg, from);
            }
            mov_arg(to, from_reg);
        }
    }
    void emit_is_unequal_based_on_tags(Label Unequal,
                                       const ArgVal &Src1,
                                       x86::Gp Reg1,
                                       const ArgVal &Src2,
                                       x86::Gp Reg2,
                                       Distance dist = dLong) {
        ERTS_CT_ASSERT(TAG_PRIMARY_IMMED1 == _TAG_PRIMARY_MASK);
        ERTS_CT_ASSERT((TAG_PRIMARY_LIST | TAG_PRIMARY_BOXED) ==
                       TAG_PRIMARY_IMMED1);
        if (always_one_of<BeamTypeId::AlwaysBoxed>(Src1)) {
            emit_is_boxed(Unequal, Reg2, dist);
        } else if (always_one_of<BeamTypeId::AlwaysBoxed>(Src2)) {
            emit_is_boxed(Unequal, Reg1, dist);
        } else if (exact_type<BeamTypeId::Cons>(Src1)) {
            emit_is_cons(Unequal, Reg2, dist);
        } else if (exact_type<BeamTypeId::Cons>(Src2)) {
            emit_is_cons(Unequal, Reg1, dist);
        } else {
            a.mov(RETd, Reg1.r32());
            a.or_(RETd, Reg2.r32());
            if (never_one_of<BeamTypeId::Cons>(Src1) ||
                never_one_of<BeamTypeId::Cons>(Src2)) {
                emit_is_boxed(Unequal, RET, dist);
            } else if (never_one_of<BeamTypeId::AlwaysBoxed>(Src1) ||
                       never_one_of<BeamTypeId::AlwaysBoxed>(Src2)) {
                emit_is_cons(Unequal, RET, dist);
            } else {
                a.and_(RETb, imm(_TAG_PRIMARY_MASK));
                a.cmp(RETb, imm(TAG_PRIMARY_IMMED1));
                if (dist == dShort) {
                    a.short_().je(Unequal);
                } else {
                    a.je(Unequal);
                }
            }
        }
    }
    void emit_are_both_immediate(const ArgVal &Src1,
                                 x86::Gp Reg1,
                                 const ArgVal &Src2,
                                 x86::Gp Reg2) {
        ERTS_CT_ASSERT(TAG_PRIMARY_IMMED1 == _TAG_PRIMARY_MASK);
        if (always_immediate(Src1)) {
            a.mov(RETd, Reg2.r32());
        } else if (always_immediate(Src2)) {
            a.mov(RETd, Reg1.r32());
        } else {
            a.mov(RETd, Reg1.r32());
            a.and_(RETd, Reg2.r32());
        }
        a.and_(RETb, imm(_TAG_PRIMARY_MASK));
        a.cmp(RETb, imm(TAG_PRIMARY_IMMED1));
    }
};
void *beamasm_metadata_insert(std::string module_name,
                              ErtsCodePtr base_address,
                              size_t code_size,
                              const std::vector<AsmRange> &ranges);
void beamasm_metadata_early_init();
void beamasm_metadata_late_init();