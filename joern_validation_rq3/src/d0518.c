zxcheck(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;

    check_op(1);
    make_bool(op, (r_has_attr(ACCESS_REF(op), a_executable) ? 1 : 0));
    return 0;
}
