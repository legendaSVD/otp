#ifndef ERL_MD5_H__
#define ERL_MD5_H__
#include <stdint.h>
#define MD5_DIGEST_LENGTH 16
typedef struct {
    uint32_t a, b, c, d;
    size_t tot_len;
    size_t len;
    uint8_t buf[64];
} erts_md5_state;
void erts_md5(const uint8_t* msg, size_t msg_len, uint8_t* out);
void erts_md5_init(erts_md5_state*);
void erts_md5_update(erts_md5_state*, const uint8_t* msg, size_t msg_len);
void erts_md5_finish(uint8_t* out, erts_md5_state*);
#endif