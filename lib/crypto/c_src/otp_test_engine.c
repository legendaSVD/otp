#ifdef _WIN32
#define OPENSSL_OPT_WINDLL
#endif
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <openssl/md5.h>
#include <openssl/rsa.h>
#define PACKED_OPENSSL_VERSION(MAJ, MIN, FIX, P)	\
    ((((((((MAJ << 8) | MIN) << 8 ) | FIX) << 8) | (P-'a'+1)) << 4) | 0xf)
#define PACKED_OPENSSL_VERSION_PLAIN(MAJ, MIN, FIX) \
    PACKED_OPENSSL_VERSION(MAJ,MIN,FIX,('a'-1))
#if OPENSSL_VERSION_NUMBER < PACKED_OPENSSL_VERSION_PLAIN(1,1,0) \
    || (defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x3050000fL)
# define OLD
#endif
#if OPENSSL_VERSION_NUMBER >= PACKED_OPENSSL_VERSION_PLAIN(1,1,0) \
    && !defined(LIBRESSL_VERSION_NUMBER)
# define FAKE_RSA_IMPL
#endif
#if OPENSSL_VERSION_NUMBER >= PACKED_OPENSSL_VERSION(0,9,8,'o') \
	&& !defined(OPENSSL_NO_EC) \
	&& !defined(OPENSSL_NO_ECDH) \
	&& !defined(OPENSSL_NO_ECDSA)
#if !defined(OPENSSL_NO_ENGINE)
# define HAVE_EC
#endif
#endif
#if defined(HAVE_EC)
#include <openssl/engine.h>
#include <openssl/pem.h>
static const char *test_engine_id = "MD5";
static const char *test_engine_name = "MD5 test engine";
#if defined(FAKE_RSA_IMPL)
static RSA_METHOD *test_rsa_method = NULL;
static int test_rsa_sign(int dtype, const unsigned char *m,
                         unsigned int m_len, unsigned char *sigret,
                         unsigned int *siglen, const RSA *rsa);
static int test_rsa_verify(int dtype, const unsigned char *m,
                           unsigned int m_len, const unsigned char *sigret,
                           unsigned int siglen, const RSA *rsa);
static int test_rsa_free(RSA *rsa);
#endif
EVP_PKEY* test_privkey_load(ENGINE *eng, const char *id, UI_METHOD *ui_method, void *callback_data);
EVP_PKEY* test_pubkey_load(ENGINE *eng, const char *id, UI_METHOD *ui_method, void *callback_data);
EVP_PKEY* test_key_load(ENGINE *er, const char *id, UI_METHOD *ui_method, void *callback_data, int priv);
static int init_test_md5(void);
static void finish_test_md5(void);
static int test_init(ENGINE *e) {
    printf("OTP Test Engine Initializatzion!\r\n");
#if defined(FAKE_RSA_IMPL)
    if ((test_rsa_method = RSA_meth_new("OTP test RSA method", 0)) == NULL) {
        fprintf(stderr, "RSA_meth_new failed\r\n");
        goto err;
    }
    if (!RSA_meth_set_finish(test_rsa_method, test_rsa_free))
        goto err;
    if (!RSA_meth_set_sign(test_rsa_method, test_rsa_sign))
        goto err;
    if (!RSA_meth_set_verify(test_rsa_method, test_rsa_verify))
        goto err;
    if (!ENGINE_set_RSA(e, test_rsa_method))
        goto err;
#endif
    if (!init_test_md5())
        goto err;
#if OPENSSL_VERSION_NUMBER < PACKED_OPENSSL_VERSION_PLAIN(1,1,0)
    OpenSSL_add_all_ciphers();
    OpenSSL_add_all_digests();
#endif
    return 111;
err:
#if defined(FAKE_RSA_IMPL)
    if (test_rsa_method)
        RSA_meth_free(test_rsa_method);
    test_rsa_method = NULL;
#endif
    return 0;
}
static int test_finish(ENGINE *e) {
    printf("OTP Test Engine Finish!\r\n");
#if defined(FAKE_RSA_IMPL)
    if (test_rsa_method) {
        RSA_meth_free(test_rsa_method);
        test_rsa_method = NULL;
    }
#endif
    finish_test_md5();
    return 111;
}
static void add_test_data(unsigned char *md, unsigned int len)
{
    unsigned int i;
    for (i=0; i<len; i++) {
        md[i] = (unsigned char)(i & 0xff);
    }
}
#if defined(FAKE_RSA_IMPL)
static int chk_test_data(const unsigned char *md, unsigned int len)
{
    unsigned int i;
    for (i=0; i<len; i++) {
        if (md[i] != (unsigned char)(i & 0xff))
            return 0;
    }
    return 1;
}
#endif
#undef data
#ifdef OLD
#define data(ctx) ((MD5_CTX *)ctx->md_data)
#endif
static int test_engine_md5_init(EVP_MD_CTX *ctx) {
    fprintf(stderr, "MD5 initialized\r\n");
#ifdef OLD
    return MD5_Init(data(ctx));
#else
    return 1;
#endif
}
static int test_engine_md5_update(EVP_MD_CTX *ctx,const void *data, size_t count)
{
    fprintf(stderr, "MD5 update\r\n");
#ifdef OLD
    return MD5_Update(data(ctx), data, (size_t)count);
#else
    return 1;
#endif
}
static int test_engine_md5_final(EVP_MD_CTX *ctx,unsigned char *md) {
#ifdef OLD
    fprintf(stderr, "MD5 final size of EVP_MD: %lu\r\n", (unsigned long)sizeof(EVP_MD));
    if (!MD5_Final(md, data(ctx)))
        goto err;
    add_test_data(md, MD5_DIGEST_LENGTH);
    return 1;
 err:
    return 0;
#else
    fprintf(stderr, "MD5 final\r\n");
    add_test_data(md, MD5_DIGEST_LENGTH);
    return 1;
#endif
}
static EVP_MD *test_engine_md5_ptr = NULL;
#ifdef OLD
static EVP_MD test_engine_md5_method=  {
        NID_md5,
        NID_undef,
        MD5_DIGEST_LENGTH,
        0,
        test_engine_md5_init,
        test_engine_md5_update,
        test_engine_md5_final,
        NULL,
        NULL,
        EVP_PKEY_NULL_method,
        MD5_CBLOCK,
        sizeof(EVP_MD *) + sizeof(MD5_CTX),
# if OPENSSL_VERSION_NUMBER >= PACKED_OPENSSL_VERSION_PLAIN(1,0,0)
        NULL,
# endif
};
#endif
static int init_test_md5(void)
{
#ifdef OLD
    test_engine_md5_ptr = &test_engine_md5_method;
#else
    EVP_MD *md;
    if ((md = EVP_MD_meth_new(NID_md5, NID_undef)) == NULL)
        return 0;
    EVP_MD_meth_set_result_size(md, MD5_DIGEST_LENGTH);
    EVP_MD_meth_set_flags(md, 0);
    EVP_MD_meth_set_init(md, test_engine_md5_init);
    EVP_MD_meth_set_update(md, test_engine_md5_update);
    EVP_MD_meth_set_final(md, test_engine_md5_final);
    EVP_MD_meth_set_copy(md, NULL);
    EVP_MD_meth_set_cleanup(md, NULL);
    EVP_MD_meth_set_input_blocksize(md, MD5_CBLOCK);
    EVP_MD_meth_set_app_datasize(md, sizeof(EVP_MD *) + sizeof(MD5_CTX));
    EVP_MD_meth_set_ctrl(md, NULL);
    test_engine_md5_ptr = md;
#endif
    return 1;
}
static void finish_test_md5(void)
{
#ifndef OLD
    if (test_engine_md5_ptr) {
        EVP_MD_meth_free(test_engine_md5_ptr);
        test_engine_md5_ptr = NULL;
    }
#endif
}
static int test_digest_ids[] = {NID_md5};
static int test_engine_digest_selector(ENGINE *e, const EVP_MD **digest,
        const int **nids, int nid) {
    if (!digest) {
        *nids = test_digest_ids;
        fprintf(stderr, "Digest is empty! Nid:%d\r\n", nid);
        return sizeof(test_digest_ids) / sizeof(*test_digest_ids);
    }
    fprintf(stderr, "Digest no %d requested\r\n",nid);
    if (nid == NID_md5) {
        *digest = test_engine_md5_ptr;
    }
    else {
        goto err;
    }
    return 1;
 err:
    *digest = NULL;
    return 0;
}
static int bind_helper(ENGINE * e, const char *id)
{
    if (!ENGINE_set_id(e, test_engine_id))
        goto err;
    if (!ENGINE_set_name(e, test_engine_name))
        goto err;
    if (!ENGINE_set_init_function(e, test_init))
        goto err;
    if (!ENGINE_set_finish_function(e, test_finish))
        goto err;
    if (!ENGINE_set_digests(e, &test_engine_digest_selector))
        goto err;
    if (!ENGINE_set_load_privkey_function(e, &test_privkey_load))
        goto err;
    if (!ENGINE_set_load_pubkey_function(e, &test_pubkey_load))
        goto err;
    return 1;
 err:
    return 0;
}
IMPLEMENT_DYNAMIC_CHECK_FN();
IMPLEMENT_DYNAMIC_BIND_FN(bind_helper);
int pem_passwd_cb_fun(char *buf, int size, int rwflag, void *password);
EVP_PKEY* test_privkey_load(ENGINE *eng, const char *id, UI_METHOD *ui_method, void *callback_data) {
    return test_key_load(eng, id, ui_method, callback_data, 1);
}
EVP_PKEY* test_pubkey_load(ENGINE *eng, const char *id, UI_METHOD *ui_method, void *callback_data) {
    return test_key_load(eng, id, ui_method, callback_data, 0);
}
EVP_PKEY* test_key_load(ENGINE *eng, const char *id, UI_METHOD *ui_method, void *callback_data, int priv)
{
    EVP_PKEY *pkey = NULL;
    FILE *f = fopen(id, "r");
    fprintf(stderr, "%s:%d test_key_load(id=%s,priv=%d)\r\n", __FILE__,__LINE__,id, priv);
    if (!f) {
        fprintf(stderr, "%s:%d fopen(%s) failed\r\n", __FILE__,__LINE__,id);
        return NULL;
    }
    pkey =
        priv
        ? PEM_read_PrivateKey(f, NULL, pem_passwd_cb_fun, callback_data)
        : PEM_read_PUBKEY(f, NULL, NULL, NULL);
    fclose(f);
    if (!pkey) {
        fprintf(stderr, "%s:%d Key read from file %s failed.\r\n", __FILE__,__LINE__,id);
        if (callback_data)
            fprintf(stderr, "Pwd = \"%s\".\r\n", (char *)callback_data);
        fprintf(stderr, "Contents of file \"%s\":\r\n",id);
        f = fopen(id, "r");
        {
            int c;
            while (!feof(f)) {
                switch (c=fgetc(f)) {
                case '\n':
                case '\r': putc('\r',stderr); putc('\n',stderr); break;
                default: putc(c, stderr);
                }
            }
        }
        fprintf(stderr, "File contents printed.\r\n");
        fclose(f);
        return NULL;
    }
    return pkey;
}
int pem_passwd_cb_fun(char *buf, int size, int rwflag, void *password)
{
    size_t i;
    if (size < 0)
        return 0;
    fprintf(stderr, "In pem_passwd_cb_fun\r\n");
    if (!password)
        return 0;
    i = strlen(password);
    if (i >= (size_t)size || i > INT_MAX - 1)
        goto err;
    fprintf(stderr, "Got FULL pwd %zu(%d) chars\r\n", i, size);
    memcpy(buf, (char*)password, i+1);
    return (int)i+1;
 err:
    fprintf(stderr, "Got TO LONG pwd %zu(%d) chars\r\n", i, size);
    return 0;
}
#if defined(FAKE_RSA_IMPL)
static unsigned char fake_flag[] = {255,3,124,180,35,10,180,151,101,247,62,59,80,122,220,
                             142,24,180,191,34,51,150,112,27,43,142,195,60,245,213,80,179};
int test_rsa_sign(int dtype,
                  const unsigned char *m, unsigned int m_len,
                  unsigned char *sigret, unsigned int *siglen,
                  const RSA *rsa)
{
    fprintf(stderr, "test_rsa_sign (dtype=%i) called m_len=%u *siglen=%u\r\n", dtype, m_len, *siglen);
    if (!sigret) {
        fprintf(stderr, "sigret = NULL\r\n");
        goto err;
    }
    if ((sizeof(fake_flag) == m_len)
        && memcmp(m,fake_flag,m_len) == 0) {
        int slen;
        printf("To be faked\r\n");
        if ((slen = RSA_size(rsa)) < 0)
            goto err;
        add_test_data(sigret, (unsigned int)slen);
        *siglen = (unsigned int)slen;
        return 1;
    }
    return 0;
 err:
    return -1;
}
int test_rsa_verify(int dtype,
                    const unsigned char *m, unsigned int m_len,
                    const unsigned char *sigret, unsigned int siglen,
                    const RSA *rsa)
{
    printf("test_rsa_verify (dtype=%i) called m_len=%u siglen=%u\r\n", dtype, m_len, siglen);
    if ((sizeof(fake_flag) == m_len)
        && memcmp(m,fake_flag,m_len) == 0) {
        int size;
        if ((size = RSA_size(rsa)) < 0)
            return 0;
        printf("To be faked\r\n");
        return (siglen == (unsigned int)size)
            && chk_test_data(sigret, siglen);
    }
    return 0;
}
static int test_rsa_free(RSA *rsa)
{
    printf("test_rsa_free called\r\n");
    return 1;
}
#endif
#endif