#ifndef HAVE_UNISTD_H
#  define HAVE_UNISTD_H 0
#  define HAVE_UNISTD_H__UNDEF
#endif
#ifndef HAVE_STDARG_H
#  define HAVE_STDARG_H 0
#  define HAVE_STDARG_H__UNDEF
#endif
#include <zlib.h>
#ifdef HAVE_UNISTD_H__UNDEF
#  undef HAVE_UNISTD_H
#  undef HAVE_UNISTD_H__UNDEF
#endif
#ifdef HAVE_STDARG_H__UNDEF
#  undef HAVE_STDARG_H
#  undef HAVE_STDARG_H__UNDEF
#endif
#define erl_zlib_alloc_init(s)              \
  do {               \
    (s)->zalloc = erl_zlib_zalloc_callback; \
    (s)->zfree = erl_zlib_zfree_callback;   \
  } while (0)
int ZEXPORT erl_zlib_deflate_start(z_stream *streamp, const Bytef* source,
				   uLong sourceLen, int level);
int ZEXPORT erl_zlib_deflate_chunk(z_stream *streamp, Bytef* dest, uLongf* destLen);
int ZEXPORT erl_zlib_deflate_finish(z_stream *streamp);
int ZEXPORT erl_zlib_inflate_start(z_stream *streamp, const Bytef* source,
                                   uLong sourceLen);
int ZEXPORT erl_zlib_inflate_chunk(z_stream *streamp, Bytef* dest, uLongf* destLen);
int ZEXPORT erl_zlib_inflate_finish(z_stream *streamp);
#define erl_zlib_compress(dest,destLen,source,sourceLen) \
    erl_zlib_compress2(dest,destLen,source,sourceLen,Z_DEFAULT_COMPRESSION)
int ZEXPORT erl_zlib_compress2 (Bytef* dest, uLongf* destLen,
				const Bytef* source, uLong sourceLen,
				int level);
int ZEXPORT erl_zlib_uncompress (Bytef* dest, uLongf* destLen,
				 const Bytef* source, uLong sourceLen);
voidpf erl_zlib_zalloc_callback (voidpf,unsigned,unsigned);
void erl_zlib_zfree_callback (voidpf,voidpf);