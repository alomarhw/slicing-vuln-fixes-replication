int wc_ecc_shared_secret_gen(ecc_key* private_key, ecc_point* point,
                                                    byte* out, word32 *outlen)
{
    int err;
    DECLARE_CURVE_SPECS(2)

    if (private_key == NULL || point == NULL || out == NULL ||
                                                            outlen == NULL) {
        return BAD_FUNC_ARG;
    }

    /* load curve info */
    err = wc_ecc_curve_load(private_key->dp, &curve,
        (ECC_CURVE_FIELD_PRIME | ECC_CURVE_FIELD_AF));
    if (err != MP_OKAY)
        return err;

#if defined(WOLFSSL_ASYNC_CRYPT) && defined(WC_ASYNC_ENABLE_ECC)
    if (private_key->asyncDev.marker == WOLFSSL_ASYNC_MARKER_ECC) {
        err = wc_ecc_shared_secret_gen_async(private_key, point,
            out, outlen, curve);
    }
    else
#endif
    {
        err = wc_ecc_shared_secret_gen_sync(private_key, point,
            out, outlen, curve);
    }

    wc_ecc_curve_free(curve);

    return err;
}
