#ifdef __WIN32__
#include <windows.h>
#else
#include <sys/select.h>
#endif
#include <errno.h>
#include <stdio.h>
#include "erl_driver.h"
#define get_int32(s) ((((unsigned char*) (s))[0] << 24) | \
                      (((unsigned char*) (s))[1] << 16) | \
                      (((unsigned char*) (s))[2] << 8)  | \
                      (((unsigned char*) (s))[3]))
#define ERTS_TEST_SCHEDULING_DRV_NAME "scheduling_drv"
#define ERTS_TEST_SCHEDULING_DRV_FLAGS  \
  ERL_DRV_FLAG_USE_PORT_LOCKING | ERL_DRV_FLAG_SOFT_BUSY
ErlDrvData start(ErlDrvPort port, char *command);
void output(ErlDrvData drv_data, char *buf, ErlDrvSizeT len);
ErlDrvSSizeT control(ErlDrvData drv_data, unsigned int command, char *buf,
		     ErlDrvSizeT len, char **rbuf, ErlDrvSizeT rlen);
void stop(ErlDrvData drv_data);
void timeout(ErlDrvData drv_data);
static void delay(unsigned ms);
static ErlDrvEntry busy_drv_entry = {
    NULL ,
    start,
    stop,
    output,
    NULL ,
    NULL ,
    ERTS_TEST_SCHEDULING_DRV_NAME,
    NULL ,
    NULL ,
    control,
    timeout,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    ERL_DRV_EXTENDED_MARKER,
    ERL_DRV_EXTENDED_MAJOR_VERSION,
    ERL_DRV_EXTENDED_MINOR_VERSION,
    ERTS_TEST_SCHEDULING_DRV_FLAGS,
    NULL ,
    NULL ,
    NULL
};
#define DBG(data,FMT)
typedef struct SchedDrvData {
  ErlDrvPort port;
  char data[255];
  int curr;
  int use_auto_busy;
} SchedDrvData;
DRIVER_INIT(busy_drv)
{
    return &busy_drv_entry;
}
ErlDrvData start(ErlDrvPort port, char *command)
{
  SchedDrvData *d = driver_alloc(sizeof(SchedDrvData));
  d->port = port;
  d->curr = 0;
  d->use_auto_busy = 0;
  DBG(d,"start\r\n");
  return (ErlDrvData) d;
}
void stop(ErlDrvData drv_data) {
  SchedDrvData *d = (SchedDrvData*)drv_data;
  driver_output(d->port,d->data,d->curr);
  DBG(d,"close\r\n");
  driver_free(d);
  return;
}
void timeout(ErlDrvData drv_data) {
  SchedDrvData *d = (SchedDrvData*)drv_data;
  set_busy_port(d->port, 0);
  DBG(d,"timeout\r\n");
}
void output(ErlDrvData drv_data, char *buf, ErlDrvSizeT len)
{
  int res;
  unsigned int command = *buf;
  SchedDrvData *d = (SchedDrvData*)drv_data;
  switch (command) {
  case 'B':
    DBG(d,"busy: ");
    set_busy_port(d->port, 1);
    break;
  case 'L':
    DBG(d,"long: ");
    delay(buf[5]*100);
    set_busy_port(d->port, 1);
    break;
  case 'D':
    DBG(d,"delay: ");
    delay(buf[5]*100);
    break;
  case 'N':
    DBG(d,"not");
    set_busy_port(d->port, 0);
    goto done;
  case 'C':
    DBG(d,"chang: ");
    break;
  case 'G':
    DBG(d,"get : ");
    driver_output(d->port,d->data,d->curr);
    return;
  default:
    driver_failure_posix((ErlDrvPort) drv_data, EINVAL);
    break;
  }
  if (len > 1) {
    unsigned int val = get_int32(buf+1);
    fprintf(stderr,"%u",val);
    d->data[d->curr++] = val;
  }
 done:
 fprintf(stderr,"\r\n");
}
ErlDrvSSizeT control(ErlDrvData drv_data, unsigned int command, char *buf,
		     ErlDrvSizeT len, char **rbuf, ErlDrvSizeT rlen)
{
    switch (command) {
    case 'B':
	set_busy_port((ErlDrvPort) drv_data, 1);
	break;
    case 'N':
	set_busy_port((ErlDrvPort) drv_data, 0);
	break;
    default:
	driver_failure_posix((ErlDrvPort) drv_data, EINVAL);
	break;
    }
    return 0;
}
static void delay(unsigned ms)
{
  fprintf(stderr,"delay(%u)",ms);
#ifdef __WIN32__
  Sleep(ms);
#else
  struct timeval t;
  t.tv_sec = ms/1000;
  t.tv_usec = (ms % 1000) * 1000;
  select(0, NULL, NULL, NULL, &t);
#endif
}