#ifndef __PACKET_PARSER_H__
#define __PACKET_PARSER_H__
#include <erl_driver.h>
#include "sys.h"
enum PacketParseType {
    TCP_PB_RAW      = 0,
    TCP_PB_1        = 1,
    TCP_PB_2        = 2,
    TCP_PB_4        = 3,
    TCP_PB_ASN1     = 4,
    TCP_PB_RM       = 5,
    TCP_PB_CDR      = 6,
    TCP_PB_FCGI     = 7,
    TCP_PB_LINE_LF  = 8,
    TCP_PB_TPKT     = 9,
    TCP_PB_HTTP     = 10,
    TCP_PB_HTTPH    = 11,
    TCP_PB_SSL_TLS  = 12,
    TCP_PB_HTTP_BIN = 13,
    TCP_PB_HTTPH_BIN = 14
};
typedef struct http_atom {
    struct http_atom* next;
    unsigned long h;
    const char* name;
    int   len;
    int index;
    ErlDrvTermData atom;
} http_atom_t;
typedef struct {
    enum {
        URI_STAR,
        URI_STRING,
        URI_ABS_PATH,
        URI_SCHEME,
        URI_HTTP,
        URI_HTTPS
    } type;
    const char* s1_ptr;
    int s1_len;
    const char* s2_ptr;
    int s2_len;
    int port;
}PacketHttpURI;
typedef int HttpResponseMessageFn(void* arg, int major, int minor, int status,
				  const char* phrase, int phrase_len);
typedef int HttpRequestMessageFn(void* arg, const http_atom_t* meth, const char* meth_ptr,
				 int meth_len, const PacketHttpURI*, int major, int minor);
typedef int HttpEohMessageFn(void *arg);
typedef int HttpHeaderMessageFn(void* arg, const http_atom_t* name,
				const char* name_ptr, int name_len,
				const char* oname_ptr, int oname_len,
				const char* value_ptr, int value_len);
typedef int HttpErrorMessageFn(void* arg, const char* buf, int len);
typedef int SslTlsFn(void* arg, unsigned type, unsigned major, unsigned minor,
                     const char* data, int len, const char* prefix, int plen);
typedef struct {
    HttpResponseMessageFn* http_response;
    HttpRequestMessageFn* http_request;
    HttpEohMessageFn* http_eoh;
    HttpHeaderMessageFn* http_header;
    HttpErrorMessageFn* http_error;
    SslTlsFn* ssl_tls;
}PacketCallbacks;
void packet_parser_init(void);
int packet_get_length(enum PacketParseType htype,
		      const char* ptr, unsigned n,
		      unsigned max_plen,
		      unsigned trunc_len,
		      char     delimiter,
		      int*     statep);
ERTS_GLB_INLINE
void packet_get_body(enum PacketParseType htype,
                     const char** bufp,
                     int* lenp);
ERTS_GLB_INLINE
int packet_parse(enum PacketParseType htype,
		 const char* buf, int len,
		 int* statep, PacketCallbacks* pcb, void* arg);
#define FCGI_VERSION_1 1
struct fcgi_head {
    unsigned char version;
    unsigned char type;
    unsigned char requestIdB1;
    unsigned char requestIdB0;
    unsigned char contentLengthB1;
    unsigned char contentLengthB0;
    unsigned char paddingLength;
    unsigned char reserved;
};
int packet_parse_http(const char*, int, int*, PacketCallbacks*, void*);
int packet_parse_ssl(const char*, int, PacketCallbacks*, void*);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE
void packet_get_body(enum PacketParseType htype, const char** bufp, int* lenp)
{
    switch (htype) {
    case TCP_PB_1:  *bufp += 1; *lenp -= 1; break;
    case TCP_PB_2:  *bufp += 2; *lenp -= 2; break;
    case TCP_PB_4:  *bufp += 4; *lenp -= 4; break;
    case TCP_PB_FCGI:
	*lenp -= ((struct fcgi_head*)*bufp)->paddingLength;
        break;
    default:
        ;
    }
}
ERTS_GLB_INLINE
int packet_parse(enum PacketParseType htype, const char* buf, int len,
		 int* statep, PacketCallbacks* pcb, void* arg)
{
    switch (htype) {
    case TCP_PB_HTTP:
    case TCP_PB_HTTPH:
    case TCP_PB_HTTP_BIN:
    case TCP_PB_HTTPH_BIN:
        if (packet_parse_http(buf, len, statep, pcb, arg) < 0)
            pcb->http_error(arg, buf, len);
        return 1;
    case TCP_PB_SSL_TLS:
	return packet_parse_ssl(buf, len, pcb, arg);
    default:;
    }
    return 0;
}
#endif
#endif