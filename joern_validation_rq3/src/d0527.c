static int kwajd_error(struct mskwaj_decompressor *base)
{
    struct mskwaj_decompressor_p *self = (struct mskwaj_decompressor_p *) base;
    return (self) ? self->error : MSPACK_ERR_ARGS;
}
