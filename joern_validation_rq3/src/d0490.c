put_queue_stats(struct queue_stats_cbdata *cbdata, uint32_t queue_id,
                const struct netdev_queue_stats *stats)
{
    struct ofputil_queue_stats oqs;

    oqs.port_no = cbdata->ofport->pp.port_no;
    oqs.queue_id = queue_id;
    oqs.tx_bytes = stats->tx_bytes;
    oqs.tx_packets = stats->tx_packets;
    oqs.tx_errors = stats->tx_errors;
    if (stats->created != LLONG_MIN) {
        calc_duration(stats->created, cbdata->now,
                      &oqs.duration_sec, &oqs.duration_nsec);
    } else {
        oqs.duration_sec = oqs.duration_nsec = UINT32_MAX;
    }
    ofputil_append_queue_stat(&cbdata->replies, &oqs);
}
