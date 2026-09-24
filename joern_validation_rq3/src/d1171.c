virtio_gpu_fill_display_info(VirtIOGPU *g,
                             struct virtio_gpu_resp_display_info *dpy_info)
{
    int i;

    for (i = 0; i < g->conf.max_outputs; i++) {
        if (g->enabled_output_bitmask & (1 << i)) {
            dpy_info->pmodes[i].enabled = 1;
            dpy_info->pmodes[i].r.width = g->req_state[i].width;
            dpy_info->pmodes[i].r.height = g->req_state[i].height;
        }
    }
}
