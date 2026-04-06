#ifndef E_OPENSSL_VERSION_H__
#define E_OPENSSL_VERSION_H__ 1
#include <openssl/opensslv.h>
#ifdef LIBRESSL_VERSION_NUMBER
# define HAS_LIBRESSL
# define HAS_LIBRESSL_VSN LIBRESSL_VERSION_NUMBER
#else
# define HAS_LIBRESSL_VSN 0
#endif
#if !defined(HAS_LIBRESSL) && \
    defined(OPENSSL_VERSION_MAJOR) && \
    (OPENSSL_VERSION_MAJOR >= 3)
# define PACKED_OPENSSL_VERSION(MAJ, MIN, PATCH, VOID)   \
         (((((MAJ << 8) | MIN) << 16 ) | PATCH) << 4)
#else
#  define PACKED_OPENSSL_VERSION(MAJ, MIN, FIX, P)                        \
          ((((((((MAJ << 8) | MIN) << 8 ) | FIX) << 8) | (P-'a'+1)) << 4) | 0xf)
#endif
#define PACKED_OPENSSL_VERSION_PLAIN(MAJ, MIN, FIX) \
    PACKED_OPENSSL_VERSION(MAJ,MIN,FIX,('a'-1))
#endif