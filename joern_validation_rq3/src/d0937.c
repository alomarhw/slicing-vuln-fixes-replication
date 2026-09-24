static int alloc_seg_map(VP9_COMMON *cm, int seg_map_size) {
 int i;

 for (i = 0; i < NUM_PING_PONG_BUFFERS; ++i) {
    cm->seg_map_array[i] = (uint8_t *)vpx_calloc(seg_map_size, 1);
 if (cm->seg_map_array[i] == NULL)
 return 1;
 }
  cm->seg_map_alloc_size = seg_map_size;

  cm->seg_map_idx = 0;
  cm->prev_seg_map_idx = 1;

  cm->current_frame_seg_map = cm->seg_map_array[cm->seg_map_idx];
 if (!cm->frame_parallel_decode)
    cm->last_frame_seg_map = cm->seg_map_array[cm->prev_seg_map_idx];

 return 0;
}
