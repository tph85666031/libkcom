#ifndef __KCOM_CRYPTO_H__
#define __KCOM_CRYPTO_H__

/*
    support name:
    sha128
    sha256
    sha512
    md5
*/
int kcom_crypto_hash(const char* name, const unsigned char* data, int data_size, unsigned char* result, int result_size);
int kcom_crypto_encrypt(const char* name,
                        const void* key, int key_size,
                        const void* iv, int iv_size,
                        const void* data, int data_size,
                        unsigned char* out, int out_size);
int kcom_crypto_decrypt(const char* name,
                        const void* key, int key_size,
                        const void* iv, int iv_size,
                        const void* data, int data_size,
                        unsigned char* out, int out_size);

#endif /* __KCOM_CRYPTO_H__ */

