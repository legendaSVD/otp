#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libpq-fe.h>
#include <erl_driver.h>
#include <ei.h>
#include "pg_encode.h"
static ErlDrvData start(ErlDrvPort port, char *command);
static void stop(ErlDrvData drv_data);
static int control(ErlDrvData drv_data, unsigned int command, char *buf,
		   int len, char **rbuf, int rlen);
static ErlDrvEntry pq_driver_entry = {
    NULL,
    start,
    stop,
    NULL,
    NULL,
    NULL,
    "pg_sync",
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
} our_data_t;
#define DRV_CONNECT             'C'
#define DRV_DISCONNECT          'D'
#define DRV_SELECT              'S'
DRIVER_INIT(pq_drv)
{
    return &pq_driver_entry;
}
static ErlDrvData start(ErlDrvPort port, char *command)
{
    our_data_t* data;
    data = (our_data_t*)driver_alloc(sizeof(our_data_t));
    data->conn = NULL;
    set_port_control_flags(port, PORT_CONTROL_FLAG_BINARY);
    return (ErlDrvData)data;
}
static int do_disconnect(our_data_t* data, ei_x_buff* x);
static void stop(ErlDrvData drv_data)
{
    do_disconnect((our_data_t*)drv_data, NULL);
}
static ErlDrvBinary* ei_x_to_new_binary(ei_x_buff* x)
{
    ErlDrvBinary* bin = driver_alloc_binary(x->index);
    if (bin != NULL)
	memcpy(&bin->orig_bytes[0], x->buff, x->index);
    return bin;
}
static char* get_s(const char* buf, int len);
static int do_connect(const char *s, our_data_t* data, ei_x_buff* x);
static int do_select(const char* s, our_data_t* data, ei_x_buff* x);
static int control(ErlDrvData drv_data, unsigned int command, char *buf,
		   int len, char **rbuf, int rlen)
{
    int r;
    ei_x_buff x;
    our_data_t* data = (our_data_t*)drv_data;
    char* s = get_s(buf, len);
    ei_x_new_with_version(&x);
    switch (command) {
        case DRV_CONNECT:    r = do_connect(s, data, &x);  break;
        case DRV_DISCONNECT: r = do_disconnect(data, &x);  break;
        case DRV_SELECT:     r = do_select(s, data, &x);   break;
        default:             r = -1;	break;
    }
    *rbuf = (char*)ei_x_to_new_binary(&x);
    ei_x_free(&x);
    driver_free(s);
    return r;
}
static int do_connect(const char *s, our_data_t* data, ei_x_buff* x)
{
    PGconn* conn = PQconnectdb(s);
    if (PQstatus(conn) != CONNECTION_OK) {
        encode_error(x, conn);
	PQfinish(conn);
	conn = NULL;
    } else {
        encode_ok(x);
    }
    data->conn = conn;
    return 0;
}
static int do_disconnect(our_data_t* data, ei_x_buff* x)
{
    if (data->conn == NULL)
	return 0;
    PQfinish(data->conn);
    data->conn = NULL;
    if (x != NULL)
	encode_ok(x);
    return 0;
}
static int do_select(const char* s, our_data_t* data, ei_x_buff* x)
{
    PGresult* res = PQexec(data->conn, s);
    encode_result(x, res, data->conn);
    PQclear(res);
    return 0;
}
static char* get_s(const char* buf, int len)
{
    char* result;
    if (len < 1 || len > 10000) return NULL;
    result = driver_alloc(len+1);
    memcpy(result, buf, len);
    result[len] = '\0';
    return result;
}