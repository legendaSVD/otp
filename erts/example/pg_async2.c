#include <erl_driver.h>
#include <libpq-fe.h>
#include <ei.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pg_encode2.h"
#define L     fprintf(stderr, "%d\r\n", __LINE__)
static ErlDrvData start(ErlDrvPort port, char *command);
static void stop(ErlDrvData drv_data);
static int control(ErlDrvData drv_data, unsigned int command, char *buf,
		   int len, char **rbuf, int rlen);
static void ready_input(ErlDrvData drv_data, ErlDrvEvent event);
static ErlDrvEntry pq_driver_entry = {
    NULL,
    start,
    stop,
    NULL,
    ready_input,
    NULL,
    "pg_async2",
    NULL,
    NULL,
    control,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
typedef struct our_data_t {
    PGconn* conn;
    ErlDrvPort port;
    int socket;
    char* s;
} our_data_t;
our_data_t our_data;
#define DRV_CONNECT             1
#define DRV_DISCONNECT          2
#define DRV_SELECT              3
#ifdef __cplusplus
extern "C" {
#endif
DRIVER_INIT(pq_drv)
{
    return &pq_driver_entry;
}
#ifdef __cplusplus
}
#endif
static ErlDrvData start(ErlDrvPort port, char *command)
{
    our_data_t* data = driver_alloc(sizeof(our_data_t));
    data->port = port;
    data->conn = NULL;
    return (ErlDrvData)data;
}
static char* get_s(const char* buf, int len);
static void free_s(char* s);
static int do_connect(const char *s, our_data_t* data);
static int do_disconnect(our_data_t* data);
static int do_select(const char* s, our_data_t* data);
static void stop(ErlDrvData drv_data)
{
    do_disconnect((our_data_t*)drv_data);
}
static int control(ErlDrvData drv_data, unsigned int command, char *buf,
		   int len, char **rbuf, int rlen)
{
    int r;
    char* s;
    s = get_s(buf, len);
    switch (command) {
    case DRV_CONNECT:
        r = do_connect(s, (our_data_t*)drv_data);
        break;
    case DRV_DISCONNECT:
	r = do_disconnect((our_data_t*)drv_data);
	break;
    case DRV_SELECT:
	r = do_select(s, (our_data_t*)drv_data);
	break;
    default:
	r = -1;
	break;
    }
    free_s(s);
    return r;
}
static int do_connect(const char *s, our_data_t* data)
{
    ei_x_buff x;
    PGconn* conn = PQconnectdb(s);
    ei_x_new_with_version(&x);
    if (PQstatus(conn) != CONNECTION_OK) {
        encode_error(&x, conn);
	PQfinish(conn);
	conn = NULL;
    } else {
        encode_ok(&x);
	data->socket = PQsocket(conn);
	driver_select(data->port, (ErlDrvEvent)data->socket, DO_READ, 1);
    }
    driver_output(data->port, x.buff, x.index);
    ei_x_free(&x);
    data->conn = conn;
    return 0;
}
static int do_disconnect(our_data_t* data)
{
    ei_x_buff x;
    if (data->socket == 0)
	return 0;
    driver_select(data->port, (ErlDrvEvent)data->socket, DO_READ, 0);
    data->socket = 0;
    PQfinish(data->conn);
    data->conn = NULL;
    ei_x_new_with_version(&x);
    encode_ok(&x);
    driver_output(data->port, x.buff, x.index);
    ei_x_free(&x);
    return 0;
}
static int do_select(const char* s, our_data_t* data)
{
    PGconn* conn = data->conn;
    if (PQsendQueryParams(conn, s, 0, NULL, NULL, NULL, NULL, 1) == 0) {
	ei_x_buff x;
	ei_x_new_with_version(&x);
	encode_error(&x, conn);
	driver_output(data->port, x.buff, x.index);
	ei_x_free(&x);
    }
    return 0;
}
static void ready_input(ErlDrvData drv_data, ErlDrvEvent event)
{
    our_data_t* data = (our_data_t*)drv_data;
    PGconn* conn = data->conn;
    ei_x_buff x;
    PGresult* res;
    PQconsumeInput(conn);
    if (PQisBusy(conn))
	return;
    ei_x_new_with_version(&x);
    res = PQgetResult(conn);
    encode_result(&x, res, conn);
    driver_output(data->port, x.buff, x.index);
    ei_x_free(&x);
    PQclear(res);
    for (;;) {
	res = PQgetResult(conn);
	if (res == NULL)
	    break;
	PQclear(res);
    }
}
static char* get_s(const char* buf, int len)
{
    char* result;
    if (len < 1 || len > 1000) return NULL;
    result = driver_alloc(len+1);
    memcpy(result, buf, len);
    result[len] = '\0';
    return result;
}
static void free_s(char* s)
{
    driver_free(s);
}