static int ebcdic_read(BIO *b, char *out, int outl)
{
    int ret = 0;

    if (out == NULL || outl == 0)
        return (0);
    if (b->next_bio == NULL)
        return (0);

    ret = BIO_read(b->next_bio, out, outl);
    if (ret > 0)
        ascii2ebcdic(out, out, ret);
    return (ret);
}
