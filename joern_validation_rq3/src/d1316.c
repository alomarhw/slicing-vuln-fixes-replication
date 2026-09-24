const char *h2_stream_state_str(h2_stream *stream)
{
    switch (stream->state) {
        case H2_STREAM_ST_IDLE:
            return "IDLE";
        case H2_STREAM_ST_OPEN:
            return "OPEN";
        case H2_STREAM_ST_RESV_LOCAL:
            return "RESERVED_LOCAL";
        case H2_STREAM_ST_RESV_REMOTE:
            return "RESERVED_REMOTE";
        case H2_STREAM_ST_CLOSED_INPUT:
            return "HALF_CLOSED_REMOTE";
        case H2_STREAM_ST_CLOSED_OUTPUT:
            return "HALF_CLOSED_LOCAL";
        case H2_STREAM_ST_CLOSED:
            return "CLOSED";
        default:
            return "UNKNOWN";
            
    }
}
