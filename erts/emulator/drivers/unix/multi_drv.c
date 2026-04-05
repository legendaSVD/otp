#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "erl_driver.h"
#include "sys.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define MAXCHANNEL 20
static char buf[BUFSIZ];
static ErlDrvData multi_start(ErlDrvPort, char*);
static int multi_init(void);
static void multi_stop(ErlDrvData),multi_erlang_read(ErlDrvData, char*, int);
struct driver_entry multi_driver_entry = {
    multi_init,
    multi_start,
    multi_stop,
    multi_erlang_read,
    NULL,
    NULL,
    "multi"
};
struct channel {
    ErlDrvPort portno;
    int channel;
};
struct channel channels[MAXCHANNEL];
static int multi_init(void)
{
    memzero(channels,MAXCHANNEL * sizeof(struct channel));
    return 0;
}
static ErlDrvData multi_start(ErlDrvPort port, char* buf)
{
    int chan;
    chan = get_new_channel();
    channels[port].portno = port;
    channels[port].channel = chan;
    fprintf(stderr,"Opening channel %d port is %d\n",chan,port);
    return (ErlDrvData)port;
}
static int multi_stop(ErlDrvData port)
{
    fprintf(stderr,"Closing channel %d\n",channels[port].channel);
    remove_channel(channels[(int)port].channel);
}
static int multi_erlang_read(ErlDrvData port, char* buf, int count)
{
    fprintf(stderr,"Writing %d bytes to channel %d\n",
	    count,
	    channels[(int)port].channel);
}
int get_new_channel()
{
    static int ch = 1;
    return(ch++);
}
void remove_channel(int ch)
{
}