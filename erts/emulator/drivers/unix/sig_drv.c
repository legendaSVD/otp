#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "sys.h"
#include "erl_driver.h"
#include <signal.h>
#include <stdio.h>
static ErlDrvData sig_start(ErlDrvPort, char*);
static int sig_init(void);
static void sig_stop(ErlDrvData), doio(ErlDrvData, ErlDrvEvent);
ErlDrvEntry sig_driver_entry = {
    sig_init,
    sig_start,
    sig_stop,
    NULL,
    doio,
    NULL,
    "sig_test"
};
static ErlDrvPort this_port;
static int sig_init(void)
{
    this_port = (ErlDrvPort)-1;
    return 0;
}
static sigc(int ino)
{
    driver_interrupt(this_port, ino);
}
static ErlDrvData sig_start(ErlDrvPort port, char* buf)
{
    if (this_port != (ErlDrvPort)-1)
	return ERL_DRV_ERROR_GENERAL;
    this_port = port;
    signal(SIGUSR1, sigc);
    return (ErlDrvData)port;
}
static void sig_stop(ErlDrvData port)
{
    this_port = (ErlDrvPort)-1;
    signal(SIGUSR1, SIG_DFL);
}
doio(ErlDrvData port, ErlDrvEvent ino)
{
    driver_output(this_port, "y", 1);
}