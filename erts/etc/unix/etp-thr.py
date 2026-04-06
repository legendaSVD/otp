def get_thread_name(t):
    f = gdb.newest_frame();
    while f:
        if f.name() == "async_main":
            return "async";
        elif f.name() == "erts_sys_main_thread":
            return "main";
        elif f.name() == "signal_dispatcher_thread_func":
            return "signal_dispatcher";
        elif f.name() == "sys_msg_dispatcher_func":
            return "sys_msg_dispatcher";
        elif f.name() == "child_waiter":
            return "child_waiter";
        elif f.name() == "sched_thread_func":
            return "scheduler";
        elif f.name() == "sched_dirty_cpu_thread_func":
            return "dirty_cpu_scheduler";
        elif f.name() == "sched_dirty_io_thread_func":
            return "dirty_io_scheduler";
        elif f.name() == "poll_thread":
            return "poll_thread";
        elif f.name() == "aux_thread":
            return "aux";
        f = f.older();
    return "unknown";
curr_thread = gdb.selected_thread();
for i in gdb.inferiors():
    gdb.write(" Id   Thread Name           Frame\n");
    for t in i.threads():
        t.switch();
        if curr_thread == t:
            gdb.write("*");
        else:
            gdb.write(" ");
        gdb.write("{0:<3}  {1:20} {2}\n".format(
                t.num,get_thread_name(t),
                gdb.newest_frame().name()));
curr_thread.switch();