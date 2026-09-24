static int in_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
 struct a2dp_stream_in *in = (struct a2dp_stream_in *)stream;

    FNLOG();

 if (in->common.cfg.rate > 0 && in->common.cfg.rate == rate)
 return 0;
 else
 return -1;
}
