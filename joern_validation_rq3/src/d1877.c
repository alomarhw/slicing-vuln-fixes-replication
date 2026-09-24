zcallbeginpage(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;
    gx_device *dev = gs_currentdevice(igs);

    check_type(*op, t_integer);
    if ((dev = (*dev_proc(dev, get_page_device))(dev)) != 0) {
        int code = (*dev->page_procs.begin_page)(dev, igs);

        if (code < 0)
            return code;
    }
    pop(1);
    return 0;
}
