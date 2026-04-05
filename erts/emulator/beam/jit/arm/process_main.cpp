#include "beam_asm.hpp"
extern "C"
{
#include "bif.h"
#include "beam_common.h"
#include "code_ix.h"
#include "export.h"
}
#undef x
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
    const a64::Mem start_time_i =
            getSchedulerRegRef(offsetof(ErtsSchedulerRegisters, start_time_i));
    const a64::Mem start_time =
            getSchedulerRegRef(offsetof(ErtsSchedulerRegisters, start_time));
    a.stp(a64::x29, a64::x30, a64::Mem(a64::sp, -16).pre());
    a.mov(TMP1, a64::sp);
    sub(TMP1, TMP1, sizeof(ErtsSchedulerRegisters) + ERTS_CACHE_LINE_SIZE);
    a.and_(TMP1, TMP1, imm(~ERTS_CACHE_LINE_MASK));
    a.mov(a64::sp, TMP1);
    a.mov(a64::x29, a64::sp);
    a.str(TMP1, a64::Mem(ARG1, offsetof(ErtsSchedulerData, registers)));
    a.mov(scheduler_registers, a64::sp);
#ifdef JIT_HARD_DEBUG
    a.mov(TMP1, a64::sp);
    a.str(TMP1, getInitialSPRef());
#endif
    a.str(a64::xzr, start_time_i);
    a.str(a64::xzr, start_time);
    mov_imm(c_p, 0);
    mov_imm(FCALLS, 0);
    mov_imm(ARG3, 0);
    a.b(schedule_next);
    a.bind(do_schedule_local);
    {
        a.ldr(TMP1, a64::Mem(c_p, offsetof(Process, def_arg_reg[5])));
        a.sub(ARG3.w(), TMP1.w(), FCALLS);
        a.b(schedule_next);
    }
    a.bind(context_switch_local);
    comment("Context switch, unknown arity/MFA");
    {
        Sint arity_offset = offsetof(ErtsCodeMFA, arity) - sizeof(ErtsCodeMFA);
        a.ldur(TMP1.w(), a64::Mem(ARG3, arity_offset));
        a.strb(TMP1.w(), a64::Mem(c_p, offsetof(Process, arity)));
        a.sub(TMP1, ARG3, imm(sizeof(ErtsCodeMFA)));
        a.str(TMP1, a64::Mem(c_p, offsetof(Process, current)));
    }
    a.bind(context_switch_simplified_local);
    comment("Context switch, known arity and MFA");
    {
        Label not_exiting = a.new_label();
#ifdef DEBUG
        Label check_i = a.new_label();
        a.tst(ARG3, imm(_CPMASK));
        a.b_eq(check_i);
        a.udf(1);
        a.bind(check_i);
#endif
        a.str(ARG3, a64::Mem(c_p, offsetof(Process, i)));
        a.ldr(TMP1.w(), a64::Mem(c_p, offsetof(Process, state.value)));
        a.tst(TMP1, imm(ERTS_PSFLG_EXITING));
        a.b_eq(not_exiting);
        {
            comment("Process exiting");
            a.adr(TMP1, labels[process_exit]);
            a.str(TMP1, a64::Mem(c_p, offsetof(Process, i)));
            a.strb(ZERO.w(), a64::Mem(c_p, offsetof(Process, arity)));
            a.str(ZERO, a64::Mem(c_p, offsetof(Process, current)));
            a.b(do_schedule_local);
        }
        a.bind(not_exiting);
        a.ldr(TMP1.w(), a64::Mem(c_p, offsetof(Process, def_arg_reg[5])));
        a.sub(FCALLS, TMP1.w(), FCALLS);
        comment("Copy out X registers");
        a.mov(ARG1, c_p);
        load_x_reg_array(ARG2);
        runtime_call<void (*)(Process *, Eterm *), copy_out_registers>();
        a.mov(ARG3.w(), FCALLS);
    }
    a.bind(schedule_next);
    comment("schedule_next");
    {
        Label schedule = a.new_label(), skip_long_schedule = a.new_label();
        a.ldr(TMP1, start_time);
        a.cbz(TMP1, schedule);
        {
            a.mov(ARG1, c_p);
            a.ldr(ARG2, start_time);
            a.str(ARG3, start_time);
            a.ldr(ARG3, start_time_i);
            runtime_call<void (*)(Process *, Uint64, ErtsCodePtr),
                         check_monitor_long_schedule>();
            a.ldr(ARG3, start_time);
        }
        a.bind(schedule);
        mov_imm(ARG1, 0);
        a.mov(ARG2, c_p);
#if defined(DEBUG) || defined(ERTS_ENABLE_LOCK_CHECK)
        runtime_call<Process *(*)(ErtsSchedulerData *, Process *, int),
                     erts_debug_schedule>();
#else
        runtime_call<Process *(*)(ErtsSchedulerData *, Process *, int),
                     erts_schedule>();
#endif
        a.mov(c_p, ARG1);
#ifdef ERTS_MSACC_EXTENDED_STATES
        lea(ARG1, erts_msacc_cache);
        runtime_call<void (*)(ErtsMsAcc **), erts_msacc_update_cache>();
#endif
        a.str(ZERO, start_time);
        mov_imm(ARG1, &erts_system_monitor_long_schedule);
        a.ldr(TMP1, a64::Mem(ARG1));
        a.cbz(TMP1, skip_long_schedule);
        {
            runtime_call<Uint64 (*)(), erts_timestamp_millis>();
            a.str(ARG1, start_time);
            a.ldr(TMP1, a64::Mem(c_p, offsetof(Process, i)));
            a.str(TMP1, start_time_i);
        }
        a.bind(skip_long_schedule);
        comment("skip_long_schedule");
        a.mov(ARG1, c_p);
        load_x_reg_array(ARG2);
        runtime_call<void (*)(Process *, Eterm *), copy_in_registers>();
        a.ldr(FCALLS, a64::Mem(c_p, offsetof(Process, fcalls)));
        a.str(FCALLS.x(), a64::Mem(c_p, offsetof(Process, def_arg_reg[5])));
#ifdef DEBUG
        a.str(FCALLS.x(), a64::Mem(c_p, offsetof(Process, debug_reds_in)));
#endif
        comment("check whether save calls is on");
        a.mov(ARG1, c_p);
        mov_imm(ARG2, ERTS_PSD_SAVED_CALLS_BUF);
        runtime_call<void *(*)(Process *, int), erts_psd_get>();
        mov_imm(TMP1, &the_active_code_index);
        a.ldr(TMP1.w(), a64::Mem(TMP1));
        a.mov(TMP2, imm(ERTS_SAVE_CALLS_CODE_IX));
        a.cmp(ARG1, ZERO);
        a.csel(active_code_ix, TMP1, TMP2, arm::CondCode::kEQ);
        emit_leave_runtime<Update::eStack | Update::eHeap | Update::eXRegs>();
        a.ldr(ARG1, a64::Mem(c_p, offsetof(Process, i)));
        a.ldr(TMP1, a64::Mem(ARG1));
        ERTS_CT_ASSERT((op_call_nif_WWW & 0xFFFF0000) == 0);
        a.cmp(TMP1, imm(op_call_nif_WWW));
        a.b_eq(labels[dispatch_nif]);
        ERTS_CT_ASSERT((op_call_bif_W & 0xFFFF0000) == 0);
        a.cmp(TMP1, imm(op_call_bif_W));
        a.b_eq(labels[dispatch_bif]);
        a.br(ARG1);
    }
    a.bind(labels[context_switch]);
    {
        emit_enter_runtime<Update::eStack | Update::eHeap | Update::eXRegs>();
        a.b(context_switch_local);
    }
    a.bind(labels[context_switch_simplified]);
    {
        emit_enter_runtime<Update::eStack | Update::eHeap | Update::eXRegs>();
        a.b(context_switch_simplified_local);
    }
    a.bind(labels[do_schedule]);
    {
        emit_enter_runtime<Update::eStack | Update::eHeap | Update::eXRegs>();
        a.b(do_schedule_local);
    }
}