zincludecolorspace(i_ctx_t * i_ctx_p)
{
    os_ptr  op = osp;
    ref nsref;
    int code;

    check_type(*op, t_name);
    name_string_ref(imemory, op, &nsref);
    code =  gs_includecolorspace(igs, nsref.value.const_bytes, r_size(&nsref));
    if (!code)
        pop(1);
    return code;
}
