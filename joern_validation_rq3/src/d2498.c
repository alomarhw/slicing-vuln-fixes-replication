static int get_next_buffer(struct resampler_buffer_provider *buffer_provider,
 struct resampler_buffer* buffer)
{
 struct stream_in *in;
 struct pcm_device *pcm_device;

 if (buffer_provider == NULL || buffer == NULL)
 return -EINVAL;

    in = (struct stream_in *)((char *)buffer_provider -
                                   offsetof(struct stream_in, buf_provider));

 if (list_empty(&in->pcm_dev_list)) {
        buffer->raw = NULL;
        buffer->frame_count = 0;
        in->read_status = -ENODEV;
 return -ENODEV;
 }

    pcm_device = node_to_item(list_head(&in->pcm_dev_list),
 struct pcm_device, stream_list_node);

 if (in->read_buf_frames == 0) {
 size_t size_in_bytes = pcm_frames_to_bytes(pcm_device->pcm, in->config.period_size);
 if (in->read_buf_size < in->config.period_size) {
            in->read_buf_size = in->config.period_size;
            in->read_buf = (int16_t *) realloc(in->read_buf, size_in_bytes);
            ALOG_ASSERT((in->read_buf != NULL),
 "get_next_buffer() failed to reallocate read_buf");
 }

        in->read_status = pcm_read(pcm_device->pcm, (void*)in->read_buf, size_in_bytes);

 if (in->read_status != 0) {
            ALOGE("get_next_buffer() pcm_read error %d", in->read_status);
            buffer->raw = NULL;
            buffer->frame_count = 0;
 return in->read_status;
 }
        in->read_buf_frames = in->config.period_size;
 }

    buffer->frame_count = (buffer->frame_count > in->read_buf_frames) ?
                                in->read_buf_frames : buffer->frame_count;
    buffer->i16 = in->read_buf + (in->config.period_size - in->read_buf_frames) *
                                                in->config.channels;
 return in->read_status;
}
