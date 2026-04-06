#ifndef _ERLSRV_SERVICE_H
#define _ERLSRV_SERVICE_H
#define CYCLIC_RESTART_LIMIT 10
#define SUCCESS_WAIT_TIME (10*1000)
#define NO_SUCCESS_WAIT 0
#define INITIAL_SUCCESS_WAIT 1
#define RESTART_SUCCESS_WAIT 2
int service_main(int argc, wchar_t **argv);
#endif