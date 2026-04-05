#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "sys.h"
#include "erl_alloc.h"
#include "erl_vm.h"
#if (defined(BEAMASM) && defined(NATIVE_ERLANG_STACK))
#if defined(NSIG)
#  define HIGHEST_SIGNAL NSIG
#elif defined(_NSIG)
#  define HIGHEST_SIGNAL _NSIG
#endif
static void sys_sigaltstack(void *ss_sp) {
    stack_t ss;
    ss.ss_sp = ss_sp;
    ss.ss_flags = 0;
    ss.ss_size = SIGSTKSZ;
    if (sigaltstack(&ss, NULL) < 0) {
        ERTS_INTERNAL_ERROR("Failed to set alternate signal stack");
    }
}
void sys_thread_init_signal_stack(void) {
    char *stack = malloc(SIGSTKSZ);
    sys_sigaltstack(stack);
}
void sys_init_signal_stack(void) {
    struct sigaction sa;
    int i;
    sys_thread_init_signal_stack();
    for (i = 1; i < HIGHEST_SIGNAL; ++i) {
        if (sigaction(i, NULL, &sa)) {
            continue;
        }
        if (sa.sa_handler == SIG_DFL || sa.sa_handler == SIG_IGN ||
            (sa.sa_flags & SA_ONSTACK)) {
            continue;
        }
        sa.sa_flags |= SA_ONSTACK;
        if (sigaction(i, &sa, NULL)) {
#ifdef SIGCANCEL
            if (i == SIGCANCEL)
                continue;
#endif
            ERTS_INTERNAL_ERROR("Failed to use alternate signal stack");
        }
    }
}
#else
void sys_init_signal_stack(void) {
}
void sys_thread_init_signal_stack(void) {
}
#endif