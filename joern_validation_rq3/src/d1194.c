static int devicenrange(i_ctx_t * i_ctx_p, ref *space, float *ptr)
{
    int i, limit, code;
    PS_colour_space_t *cspace;

    ref altspace;

    code = array_get(imemory, space, 1, &altspace);
    if (code < 0)
        return code;

    code = get_space_object(i_ctx_p, &altspace, &cspace);
    if (code < 0)
        return code;

    code = cspace->numcomponents(i_ctx_p, &altspace, &limit);
    if (code < 0)
        return code;

    for (i = 0;i < limit * 2;i+=2) {
        ptr[i] = 0;
        ptr[i+1] = 1;
    }
    return 0;
}
