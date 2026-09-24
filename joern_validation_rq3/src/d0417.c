void reds_handle_agent_mouse_event(const VDAgentMouseState *mouse_state)
{
    SpiceCharDeviceWriteBuffer *char_dev_buf;
    VDInternalBuf *internal_buf;
    uint32_t total_msg_size;

    if (!inputs_inited() || !reds->agent_state.base) {
        return;
    }

    total_msg_size = sizeof(VDIChunkHeader) + sizeof(VDAgentMessage) +
                     sizeof(VDAgentMouseState);
    char_dev_buf = spice_char_device_write_buffer_get(reds->agent_state.base,
                                                      NULL,
                                                      total_msg_size);

    if (!char_dev_buf) {
        reds->pending_mouse_event = TRUE;

        return;
    }
    reds->pending_mouse_event = FALSE;

    internal_buf = (VDInternalBuf *)char_dev_buf->buf;
    internal_buf->chunk_header.port = VDP_SERVER_PORT;
    internal_buf->chunk_header.size = sizeof(VDAgentMessage) + sizeof(VDAgentMouseState);
    internal_buf->header.protocol = VD_AGENT_PROTOCOL;
    internal_buf->header.type = VD_AGENT_MOUSE_STATE;
    internal_buf->header.opaque = 0;
    internal_buf->header.size = sizeof(VDAgentMouseState);
    internal_buf->u.mouse_state = *mouse_state;

    char_dev_buf->buf_used = total_msg_size;
    spice_char_device_write_buffer_add(reds->agent_state.base, char_dev_buf);
}
