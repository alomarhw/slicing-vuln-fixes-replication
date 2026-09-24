int ssl3_send_certificate_request(SSL *s)
{
    unsigned char *p, *d;
    int i, j, nl, off, n;
    STACK_OF(X509_NAME) *sk = NULL;
    X509_NAME *name;
    BUF_MEM *buf;

    if (s->state == SSL3_ST_SW_CERT_REQ_A) {
        buf = s->init_buf;

        d = p = ssl_handshake_start(s);

        /* get the list of acceptable cert types */
        p++;
        n = ssl3_get_req_cert_type(s, p);
        d[0] = n;
        p += n;
        n++;

        if (SSL_USE_SIGALGS(s)) {
            const unsigned char *psigs;
            nl = tls12_get_psigalgs(s, &psigs);
            s2n(nl, p);
            memcpy(p, psigs, nl);
            p += nl;
            n += nl + 2;
        }

        off = n;
        p += 2;
        n += 2;

        sk = SSL_get_client_CA_list(s);
        nl = 0;
        if (sk != NULL) {
            for (i = 0; i < sk_X509_NAME_num(sk); i++) {
                name = sk_X509_NAME_value(sk, i);
                j = i2d_X509_NAME(name, NULL);
                if (!BUF_MEM_grow_clean
                    (buf, SSL_HM_HEADER_LENGTH(s) + n + j + 2)) {
                    SSLerr(SSL_F_SSL3_SEND_CERTIFICATE_REQUEST,
                           ERR_R_BUF_LIB);
                    goto err;
                }
                p = ssl_handshake_start(s) + n;
                if (!(s->options & SSL_OP_NETSCAPE_CA_DN_BUG)) {
                    s2n(j, p);
                    i2d_X509_NAME(name, &p);
                    n += 2 + j;
                    nl += 2 + j;
                } else {
                    d = p;
                    i2d_X509_NAME(name, &p);
                    j -= 2;
                    s2n(j, d);
                    j += 2;
                    n += j;
                    nl += j;
                }
            }
        }
        /* else no CA names */
        p = ssl_handshake_start(s) + off;
        s2n(nl, p);

        ssl_set_handshake_header(s, SSL3_MT_CERTIFICATE_REQUEST, n);

#ifdef NETSCAPE_HANG_BUG
        if (!SSL_IS_DTLS(s)) {
            if (!BUF_MEM_grow_clean(buf, s->init_num + 4)) {
                SSLerr(SSL_F_SSL3_SEND_CERTIFICATE_REQUEST, ERR_R_BUF_LIB);
                goto err;
            }
            p = (unsigned char *)s->init_buf->data + s->init_num;
            /* do the header */
            *(p++) = SSL3_MT_SERVER_DONE;
            *(p++) = 0;
            *(p++) = 0;
            *(p++) = 0;
            s->init_num += 4;
        }
#endif

        s->state = SSL3_ST_SW_CERT_REQ_B;
    }

    /* SSL3_ST_SW_CERT_REQ_B */
    return ssl_do_write(s);
 err:
    s->state = SSL_ST_ERR;
    return (-1);
}
