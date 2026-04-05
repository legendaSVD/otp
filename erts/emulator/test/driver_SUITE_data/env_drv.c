#include <string.h>
#include <stdio.h>
#include "erl_driver.h"
static ErlDrvSSizeT env_drv_ctl(ErlDrvData drv_data, unsigned int cmd,
        char* buf, ErlDrvSizeT len, char** rbuf, ErlDrvSizeT rsize);
static ErlDrvEntry env_drv_entry = {
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    "env_drv",
    NULL ,
    NULL ,
    env_drv_ctl,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    ERL_DRV_EXTENDED_MARKER,
    ERL_DRV_EXTENDED_MAJOR_VERSION,
    ERL_DRV_EXTENDED_MINOR_VERSION,
    ERL_DRV_FLAG_USE_PORT_LOCKING,
    NULL ,
    NULL
};
DRIVER_INIT(env_drv) {
    return &env_drv_entry;
}
static int test_putenv(ErlDrvData drv_data, char *buf, ErlDrvSizeT len) {
    char key[256], value[256];
    int key_len, value_len;
    key_len = buf[0];
    value_len = buf[1];
    sprintf(key, "%.*s", key_len, &buf[2]);
    sprintf(value, "%.*s", value_len, &buf[2 + key_len]);
    return erl_drv_putenv(key, value);
}
static int test_getenv(ErlDrvData drv_data, char *buf, ErlDrvSizeT len) {
    char expected_value[256], stored_value[256], key[256];
    int expected_value_len, key_len;
    size_t stored_value_len;
    int res;
    key_len = buf[0];
    sprintf(key, "%.*s", key_len, &buf[2]);
    expected_value_len = buf[1];
    sprintf(expected_value, "%.*s", expected_value_len, &buf[2 + key_len]);
    stored_value_len = sizeof(stored_value);
    res = erl_drv_getenv(key, stored_value, &stored_value_len);
    if(res == 0) {
        return strcmp(stored_value, expected_value) != 0;
    } else if(res == 1) {
        return 127;
    }
    return 255;
}
static ErlDrvSSizeT env_drv_ctl(ErlDrvData drv_data, unsigned int cmd,
        char* buf, ErlDrvSizeT len, char** rbuf, ErlDrvSizeT rsize) {
    if(cmd == 0) {
        (**rbuf) = (char)test_putenv(drv_data, buf, len);
    } else {
        (**rbuf) = (char)test_getenv(drv_data, buf, len);
    }
    return 1;
}