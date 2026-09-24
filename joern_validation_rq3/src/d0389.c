static int parse_h264_sdp_line(AVFormatContext *s, int st_index,
                               PayloadContext *h264_data, const char *line)
{
    AVStream *stream;
    const char *p = line;

    if (st_index < 0)
        return 0;

    stream = s->streams[st_index];

    if (av_strstart(p, "framesize:", &p)) {
        ff_h264_parse_framesize(stream->codecpar, p);
    } else if (av_strstart(p, "fmtp:", &p)) {
        return ff_parse_fmtp(s, stream, h264_data, p, sdp_parse_fmtp_config_h264);
    } else if (av_strstart(p, "cliprect:", &p)) {
    }

    return 0;
}
