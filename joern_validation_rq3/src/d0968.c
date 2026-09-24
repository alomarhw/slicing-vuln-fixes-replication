static int get_free_fb (VP8_COMMON *cm)
{
 int i;
 for (i = 0; i < NUM_YV12_BUFFERS; i++)
 if (cm->fb_idx_ref_cnt[i] == 0)
 break;

    assert(i < NUM_YV12_BUFFERS);
    cm->fb_idx_ref_cnt[i] = 1;
 return i;
}
