void h2o_http2_conn_register_stream(h2o_http2_conn_t *conn, h2o_http2_stream_t *stream)
{
    khiter_t iter;
    int r;

    if (!h2o_http2_stream_is_push(stream->stream_id) && conn->pull_stream_ids.max_open < stream->stream_id)
        conn->pull_stream_ids.max_open = stream->stream_id;

    iter = kh_put(h2o_http2_stream_t, conn->streams, stream->stream_id, &r);
    assert(iter != kh_end(conn->streams));
    kh_val(conn->streams, iter) = stream;
}
