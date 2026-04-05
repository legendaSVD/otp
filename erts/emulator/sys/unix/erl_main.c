#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "sys.h"
#include "erl_vm.h"
#include "global.h"
int
main(int argc, char **argv)
{
    sys_init_signal_stack();
    erl_start(argc, argv);
    return 0;
}