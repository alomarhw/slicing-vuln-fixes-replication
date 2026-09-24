int tls1_final_finish_mac(SSL *s, const char *str, int slen, unsigned char *out)
{
    int hashlen;
    unsigned char hash[EVP_MAX_MD_SIZE];

    if (!ssl3_digest_cached_records(s, 0))
        return 0;

    hashlen = ssl_handshake_hash(s, hash, sizeof(hash));

    if (hashlen == 0)
        return 0;

    if (!tls1_PRF(s, str, slen, hash, hashlen, NULL, 0, NULL, 0, NULL, 0,
                  s->session->master_key, s->session->master_key_length,
                  out, TLS1_FINISH_MAC_LENGTH))
        return 0;
    OPENSSL_cleanse(hash, hashlen);
    return TLS1_FINISH_MAC_LENGTH;
}
