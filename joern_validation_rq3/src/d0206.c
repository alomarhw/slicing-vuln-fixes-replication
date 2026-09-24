static int aes_wrap_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
                             const unsigned char *iv, int enc)
{
    EVP_AES_WRAP_CTX *wctx = EVP_C_DATA(EVP_AES_WRAP_CTX,ctx);
    if (!iv && !key)
        return 1;
    if (key) {
        if (EVP_CIPHER_CTX_encrypting(ctx))
            AES_set_encrypt_key(key, EVP_CIPHER_CTX_key_length(ctx) * 8,
                                &wctx->ks.ks);
        else
            AES_set_decrypt_key(key, EVP_CIPHER_CTX_key_length(ctx) * 8,
                                &wctx->ks.ks);
        if (!iv)
            wctx->iv = NULL;
    }
    if (iv) {
        memcpy(EVP_CIPHER_CTX_iv_noconst(ctx), iv, EVP_CIPHER_CTX_iv_length(ctx));
        wctx->iv = EVP_CIPHER_CTX_iv_noconst(ctx);
    }
    return 1;
}
