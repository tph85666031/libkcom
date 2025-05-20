#include <linux/scatterlist.h>
#include <linux/crypto.h>
#include <crypto/hash.h>
#include <crypto/aes.h>
#include <crypto/skcipher.h>
#include "kcom_log.h"

int kcom_crypto_hash(const char* name, const unsigned char* data, int data_size, unsigned char* out, int out_size)
{
    if(unlikely(name == NULL || data == NULL || data_size <= 0))
    {
        return -1;
    }
    struct crypto_shash* ctx = crypto_alloc_shash(name, 0, 0);
    if(ctx == NULL)
    {
        return -2;
    }

    if(crypto_shash_digestsize(ctx) > out_size)
    {
        crypto_free_shash(ctx);
        return -3;
    }
    struct shash_desc* desc = kzalloc(sizeof(struct shash_desc) + crypto_shash_descsize(ctx), GFP_NOWAIT);
    if(desc == NULL)
    {
        crypto_free_shash(ctx);
        return -4;
    }
    desc->tfm = ctx;

    if(crypto_shash_digest(desc, data, data_size, out) == 0)
    {
        out_size = crypto_shash_digestsize(ctx);
    }
    else
    {
        out_size = 0;
    }
    crypto_free_shash(ctx);
    kfree(desc);
    return out_size;
}
EXPORT_SYMBOL(kcom_crypto_hash);

int kcom_crypto_encrypt(const char* name,
                        const void* key, int key_size,
                        const void* iv, int iv_size,
                        const void* data, int data_size,
                        unsigned char* out, int out_size)
{
    if(unlikely(name == NULL || key == NULL || key_size <= 0 || data == NULL || data_size <= 0 || out == NULL || out_size < data_size))
    {
        KLOG_E("arg incorrect");
        return -1;
    }

    struct crypto_skcipher* ctx = crypto_alloc_skcipher(name, 0, 0);
    if(IS_ERR(ctx))
    {
        KLOG_E("failed to alloc skcipher:%s", name);
        return -2;
    }

    if(key_size < crypto_skcipher_min_keysize(ctx)
            || key_size > crypto_skcipher_max_keysize(ctx)
            || iv_size != crypto_skcipher_ivsize(ctx))
    {
        KLOG_E("key_size=%d[%u-%u],iv_size=%d[%u]",
               key_size,
               crypto_skcipher_min_keysize(ctx),
               crypto_skcipher_max_keysize(ctx),
               iv_size,
               crypto_skcipher_ivsize(ctx));
        crypto_free_skcipher(ctx);
        return -3;
    }
    crypto_skcipher_setkey(ctx, key, key_size);
    int block_size = crypto_skcipher_blocksize(ctx);
    int total_size = (data_size / block_size) * block_size;

    if(data_size % block_size)
    {
        total_size += block_size;
    }

    KLOG_I("block_size=%d,total_size=%d", block_size, total_size);
    if(total_size > out_size)
    {
        crypto_free_skcipher(ctx);
        return -3;
    }

    memset(out, 0, total_size);
    memcpy(out, data, data_size);

    struct skcipher_request* req = skcipher_request_alloc(ctx, GFP_NOWAIT);
    if(IS_ERR(req))
    {
        crypto_free_skcipher(ctx);
        KLOG_E("failed to alloc request:%s", name);
        return -4;
    }

    DECLARE_CRYPTO_WAIT(wait);
    skcipher_request_set_callback(req, 0, crypto_req_done, &wait);

    struct scatterlist sg;
    sg_init_one(&sg, out, total_size);
    skcipher_request_set_crypt(req, &sg, &sg, total_size, (void*)iv);

    int ret = crypto_wait_req(crypto_skcipher_encrypt(req), &wait);
    if(ret != 0)
    {
        crypto_free_skcipher(ctx);
        skcipher_request_free(req);
        return -5;
    }

    crypto_free_skcipher(ctx);
    skcipher_request_free(req);
    return total_size;
}
EXPORT_SYMBOL(kcom_crypto_encrypt);

int kcom_crypto_decrypt(const char* name,
                        const void* key, int key_size,
                        const void* iv, int iv_size,
                        const void* data, int data_size,
                        unsigned char* out, int out_size)
{
    if(unlikely(name == NULL || key == NULL || key_size <= 0 || data == NULL || data_size <= 0 || out == NULL || out_size < data_size))
    {
        return -1;
    }
    struct crypto_skcipher* ctx = crypto_alloc_skcipher(name, 0, 0);
    if(ctx == NULL)
    {
        return -2;
    }
    int block_size = crypto_skcipher_blocksize(ctx);
    int total_size = (data_size / block_size) * block_size;

    if(data_size % block_size)
    {
        total_size += block_size;
    }

    if(total_size > out_size)
    {
        crypto_free_skcipher(ctx);
        return -3;
    }

    memset(out, 0, total_size);
    memcpy(out, data, data_size);

    struct skcipher_request* req = skcipher_request_alloc(ctx, GFP_KERNEL);
    if(req == NULL)
    {
        crypto_free_skcipher(ctx);
        return -4;
    }

    DECLARE_CRYPTO_WAIT(wait);
    skcipher_request_set_callback(req, 0, crypto_req_done, &wait);

    crypto_skcipher_setkey(ctx, key, key_size);
    struct scatterlist sg;
    sg_init_one(&sg, out, total_size);

    skcipher_request_set_crypt(req, &sg, &sg, total_size, (void*)iv);

    int ret = crypto_wait_req(crypto_skcipher_encrypt(req), &wait);
    if(ret != 0)
    {
        crypto_free_skcipher(ctx);
        skcipher_request_free(req);
        return -5;
    }

    crypto_free_skcipher(ctx);
    skcipher_request_free(req);
    return total_size;
}
EXPORT_SYMBOL(kcom_crypto_decrypt);

