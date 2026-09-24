put_queue_get_config_reply(struct ofport *port, uint32_t queue,
                           struct ovs_list *replies)
{
    struct ofputil_queue_config qc;

    /* None of the existing queues have compatible properties, so we hard-code
     * omitting min_rate and max_rate. */
    qc.port = port->ofp_port;
    qc.queue = queue;
    qc.min_rate = UINT16_MAX;
    qc.max_rate = UINT16_MAX;
    ofputil_append_queue_get_config_reply(&qc, replies);
}
