#include "beam_asm.hpp"
extern "C"
{
#include "bif.h"
#include "beam_common.h"
#include "code_ix.h"
#include "export.h"
}
const uint8_t *BeamAssembler::nops[3] = {nop1, nop2, nop3};
const uint8_t BeamAssembler::nop1[1] = {0x90};
const uint8_t BeamAssembler::nop2[2] = {0x66, 0x90};
const uint8_t BeamAssembler::nop3[3] = {0x0F, 0x1F, 0x00};
#if defined(DEBUG) || defined(ERTS_ENABLE_LOCK_CHECK)
static Process *erts_debug_schedule(ErtsSchedulerData *esdp,
                                    Process *c_p,
                                    int calls) {
    PROCESS_MAIN_CHK_LOCKS(c_p);
    ERTS_UNREQ_PROC_MAIN_LOCK(c_p);
    ERTS_VERIFY_UNUSED_TEMP_ALLOC(c_p);
    c_p = erts_schedule(esdp, c_p, calls);
    ERTS_VERIFY_UNUSED_TEMP_ALLOC(c_p);
    ERTS_REQ_PROC_MAIN_LOCK(c_p);
    PROCESS_MAIN_CHK_LOCKS(c_p);
    return c_p;
}
#endif
void BeamGlobalAssembler::emit_process_main() {
    Label context_switch_local = a.new_label(),
          context_switch_simplified_local = a.new_label(),
          do_schedule_local = a.new_label(), schedule_next = a.new_label();
    const x86::Mem start_time_i =
            getSchedulerRegRef(offsetof(ErtsSchedulerRegisters, start_time_i));
    const x86::Mem start_time =
            getSchedulerRegRef(offsetof(ErtsSchedulerRegisters, start_time));
    a.sub(x86::rsp, imm(sizeof(ErtsSchedulerRegisters) + ERTS_CACHE_LINE_SIZE));
    a.and_(x86::rsp, imm(~ERTS_CACHE_LINE_MASK));
    a.mov(x86::qword_ptr(ARG1, offsetof(ErtsSchedulerData, registers)),
          x86::rsp);
    a.lea(registers,
          x86::qword_ptr(x86::rsp,
                         offsetof(ErtsSchedulerRegisters, x_reg_array.d)));
#if defined(DEBUG) && defined(NATIVE_ERLANG_STACK)
    runtime_call<const void *(*)(void), erts_get_stacklimit>();
    a.mov(getSchedulerRegRef(
                  offsetof(ErtsSchedulerRegisters, runtime_stack_end)),
          RET);
    a.mov(getSchedulerRegRef(
                  offsetof(ErtsSchedulerRegisters, runtime_stack_start)),
          x86::rsp);
#elif !defined(NATIVE_ERLANG_STACK)
#    ifdef JIT_HARD_DEBUG
    a.mov(getInitialSPRef(), x86::rsp);
#    endif
    a.mov(getRuntimeStackRef(), x86::rsp);
    a.sub(x86::rsp, imm(15));
    a.and_(x86::rsp, imm(-16));
#endif
    a.mov(start_time_i, imm(0));
    a.mov(start_time, imm(0));
    mov_imm(c_p, 0);
    mov_imm(FCALLS, 0);
    mov_imm(ARG3, 0);
    a.jmp(schedule_next);
    a.bind(do_schedule_local);
    {
        a.mov(ARG3, x86::qword_ptr(c_p, offsetof(Process, def_arg_reg[5])));
        a.sub(ARG3d, FCALLS);
        a.jmp(schedule_next);
    }
    a.bind(context_switch_local);
    comment("Context switch, unknown arity/MFA");
    {
        Sint arity_offset = offsetof(ErtsCodeMFA, arity) - sizeof(ErtsCodeMFA);
        a.movzx(ARG1d, x86::byte_ptr(ARG3, arity_offset));
        a.mov(x86::byte_ptr(c_p, offsetof(Process, arity)), ARG1.r8());
        a.lea(ARG1, x86::qword_ptr(ARG3, -(Sint)sizeof(ErtsCodeMFA)));
        a.mov(x86::qword_ptr(c_p, offsetof(Process, current)), ARG1);
    }
    a.bind(context_switch_simplified_local);
    comment("Context switch, known arity and MFA");
    {
        Label not_exiting = a.new_label();
#ifdef ERLANG_FRAME_POINTERS
        a.sub(frame_pointer, frame_pointer);
#endif
#ifdef DEBUG
        Label check_i = a.new_label();
        a.test(ARG3, imm(_CPMASK));
        a.je(check_i);
        comment("# ARG3 is not a valid CP");
        a.ud2();
        a.bind(check_i);
#endif
        a.mov(x86::qword_ptr(c_p, offsetof(Process, i)), ARG3);
#if defined(JIT_HARD_DEBUG) && defined(ERLANG_FRAME_POINTERS)
        a.mov(ARG1, c_p);
        a.mov(ARG2, x86::qword_ptr(c_p, offsetof(Process, frame_pointer)));
        a.mov(ARG3, x86::qword_ptr(c_p, offsetof(Process, stop)));
        runtime_call<void (*)(Process *, Eterm *, Eterm *),
                     erts_validate_stack>();
#endif
#ifdef WIN32
        a.mov(ARG1d, x86::dword_ptr(c_p, offsetof(Process, state.value)));
#else
        a.mov(ARG1d, x86::dword_ptr(c_p, offsetof(Process, state.counter)));
#endif
        a.test(ARG1d, imm(ERTS_PSFLG_EXITING));
        a.short_().je(not_exiting);
        {
            comment("Process exiting");
            a.lea(ARG1, x86::qword_ptr(labels[process_exit]));
            a.mov(x86::qword_ptr(c_p, offsetof(Process, i)), ARG1);
            a.mov(x86::byte_ptr(c_p, offsetof(Process, arity)), imm(0));
            a.mov(x86::qword_ptr(c_p, offsetof(Process, current)), imm(0));
            a.jmp(do_schedule_local);
        }
        a.bind(not_exiting);
        a.mov(ARG3, x86::qword_ptr(c_p, offsetof(Process, def_arg_reg[5])));
        a.sub(ARG3d, FCALLS);
        a.mov(FCALLS, ARG3d);
        a.mov(ARG1, c_p);
        load_x_reg_array(ARG2);
        runtime_call<void (*)(Process *, Eterm *), copy_out_registers>();
        a.mov(ARG3d, FCALLS);
    }
    a.bind(schedule_next);
    comment("schedule_next");
    {
        Label schedule = a.new_label(), skip_long_schedule = a.new_label();
        a.cmp(start_time, imm(0));
        a.short_().je(schedule);
        {
            a.mov(ARG1, c_p);
            a.mov(ARG2, start_time);
            a.mov(start_time, ARG3);
            a.mov(ARG3, start_time_i);
            runtime_call<void (*)(Process *, Uint64, ErtsCodePtr),
                         check_monitor_long_schedule>();
            a.mov(ARG3, start_time);
        }
        a.bind(schedule);
#ifdef ERLANG_FRAME_POINTERS
        if (erts_frame_layout == ERTS_FRAME_LAYOUT_FP_RA) {
            a.sub(frame_pointer, frame_pointer);
        }
#endif
        mov_imm(ARG1, 0);
        a.mov(ARG2, c_p);
#if defined(DEBUG) || defined(ERTS_ENABLE_LOCK_CHECK)
        runtime_call<Process *(*)(ErtsSchedulerData *, Process *, int),
                     erts_debug_schedule>();
#else
        runtime_call<Process *(*)(ErtsSchedulerData *, Process *, int),
                     erts_schedule>();
#endif
        a.mov(c_p, RET);
#ifdef ERTS_MSACC_EXTENDED_STATES
        a.lea(ARG1,
              x86::qword_ptr(registers,
                             offsetof(ErtsSchedulerRegisters,
                                      aux_regs.d.erts_msacc_cache)));
        runtime_call<void (*)(ErtsMsAcc **), erts_msacc_update_cache>();
#endif
        a.mov(ARG1, imm((UWord)&erts_system_monitor_long_schedule));
        a.cmp(x86::qword_ptr(ARG1), imm(0));
        a.mov(start_time, imm(0));
        a.short_().je(skip_long_schedule);
        {
            runtime_call<Uint64 (*)(), erts_timestamp_millis>();
            a.mov(start_time, RET);
            a.mov(RET, x86::qword_ptr(c_p, offsetof(Process, i)));
            a.mov(start_time_i, RET);
        }
        a.bind(skip_long_schedule);
        a.mov(ARG1, c_p);
        load_x_reg_array(ARG2);
        runtime_call<void (*)(Process *, Eterm *), copy_in_registers>();
        a.mov(FCALLS, x86::dword_ptr(c_p, offsetof(Process, fcalls)));
        a.mov(x86::qword_ptr(c_p, offsetof(Process, def_arg_reg[5])),
              FCALLS.r64());
#ifdef DEBUG
        a.mov(x86::qword_ptr(c_p, offsetof(Process, debug_reds_in)),
              FCALLS.r64());
#endif
        a.mov(ARG1, c_p);
        a.mov(ARG2, imm(ERTS_PSD_SAVED_CALLS_BUF));
        runtime_call<void *(*)(Process *, int), erts_psd_get>();
        a.test(RET, RET);
        a.mov(ARG1, imm(&the_active_code_index));
        a.mov(ARG2, imm(ERTS_SAVE_CALLS_CODE_IX));
        a.mov(active_code_ix.r32(), x86::dword_ptr(ARG1));
        a.cmovnz(active_code_ix, ARG2);
        emit_leave_runtime<Update::eStack | Update::eHeap>();
        a.mov(RET, x86::qword_ptr(c_p, offsetof(Process, i)));
        a.cmp(x86::qword_ptr(RET), imm(op_call_nif_WWW));
        a.je(labels[dispatch_nif]);
        a.cmp(x86::qword_ptr(RET), imm(op_call_bif_W));
        a.je(labels[dispatch_bif]);
        a.jmp(RET);
    }
    a.bind(labels[context_switch]);
    {
        emit_enter_runtime<Update::eStack | Update::eHeap>();
        a.jmp(context_switch_local);
    }
    a.bind(labels[context_switch_simplified]);
    {
        emit_enter_runtime<Update::eStack | Update::eHeap>();
        a.jmp(context_switch_simplified_local);
    }
    a.bind(labels[do_schedule]);
    {
        emit_enter_runtime<Update::eStack | Update::eHeap>();
        a.jmp(do_schedule_local);
    }
}