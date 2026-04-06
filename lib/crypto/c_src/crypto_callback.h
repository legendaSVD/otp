#include <openssl/crypto.h>
#ifdef NEED_EVP_COMPATIBILITY_FUNCTIONS
# define CCB_FILE_LINE_ARGS
#else
# define CCB_FILE_LINE_ARGS , const char *file, int line
#endif
struct crypto_callbacks
{
    size_t sizeof_me;
    void* (*crypto_alloc)(size_t size CCB_FILE_LINE_ARGS);
    void* (*crypto_realloc)(void* ptr, size_t size CCB_FILE_LINE_ARGS);
    void (*crypto_free)(void* ptr CCB_FILE_LINE_ARGS);
#if OPENSSL_VERSION_NUMBER < 0x10100000
  #ifdef OPENSSL_THREADS
    int (*add_lock_function)(int *num, int amount, int type,
			     const char *file, int line);
    void (*locking_function)(int mode, int n, const char *file, int line);
    unsigned long (*id_function)(void);
    struct CRYPTO_dynlock_value* (*dyn_create_function)(const char *file,
							int line);
    void (*dyn_lock_function)(int mode, struct CRYPTO_dynlock_value* ptr,
			      const char *file, int line);
    void (*dyn_destroy_function)(struct CRYPTO_dynlock_value *ptr,
				 const char *file, int line);
  #endif
#endif
};
typedef struct crypto_callbacks* get_crypto_callbacks_t(int nlocks);
#ifndef HAVE_DYNAMIC_CRYPTO_LIB
struct crypto_callbacks* get_crypto_callbacks(int nlocks);
#endif