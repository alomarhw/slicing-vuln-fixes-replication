static void update_input_window(h2o_http2_conn_t *conn, uint32_t stream_id, h2o_http2_window_t *window, size_t consumed)
{
    h2o_http2_window_consume_window(window, consumed);
    if (h2o_http2_window_get_window(window) * 2 < H2O_HTTP2_SETTINGS_HOST.initial_window_size) {
        int32_t delta = (int32_t)(H2O_HTTP2_SETTINGS_HOST.initial_window_size - h2o_http2_window_get_window(window));
        h2o_http2_encode_window_update_frame(&conn->_write.buf, stream_id, delta);
        h2o_http2_conn_request_write(conn);
        h2o_http2_window_update(window, delta);
    }
}
