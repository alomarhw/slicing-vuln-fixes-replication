SPICE_GNUC_VISIBLE int spice_server_set_agent_file_xfer(SpiceServer *s, int enable)
{
    spice_assert(reds == s);
    agent_file_xfer = enable;
    reds->agent_state.write_filter.file_xfer_enabled = agent_file_xfer;
    reds->agent_state.read_filter.file_xfer_enabled = agent_file_xfer;
    return 0;
}
