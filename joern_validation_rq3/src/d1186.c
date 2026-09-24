    OVS_EXCLUDED(ofproto_mutex)
{
    struct ofproto *ofproto = ofconn_get_ofproto(ofconn);
    struct ofputil_flow_stats_request request;
    struct ofputil_aggregate_stats stats;
    bool unknown_packets, unknown_bytes;
    struct rule_criteria criteria;
    struct rule_collection rules;
    struct ofpbuf *reply;
    enum ofperr error;

    error = ofputil_decode_flow_stats_request(&request, oh,
                                              ofproto_get_tun_tab(ofproto),
                                              &ofproto->vl_mff_map);
    if (error) {
        return error;
    }

    rule_criteria_init(&criteria, request.table_id, &request.match, 0,
                       OVS_VERSION_MAX, request.cookie, request.cookie_mask,
                       request.out_port, request.out_group);

    ovs_mutex_lock(&ofproto_mutex);
    error = collect_rules_loose(ofproto, &criteria, &rules);
    rule_criteria_destroy(&criteria);
    if (!error) {
        rule_collection_ref(&rules);
    }
    ovs_mutex_unlock(&ofproto_mutex);

    if (error) {
        return error;
    }

    memset(&stats, 0, sizeof stats);
    unknown_packets = unknown_bytes = false;

    struct rule *rule;
    RULE_COLLECTION_FOR_EACH (rule, &rules) {
        uint64_t packet_count;
        uint64_t byte_count;
        long long int used;

        ofproto->ofproto_class->rule_get_stats(rule, &packet_count,
                                               &byte_count, &used);

        if (packet_count == UINT64_MAX) {
            unknown_packets = true;
        } else {
            stats.packet_count += packet_count;
        }

        if (byte_count == UINT64_MAX) {
            unknown_bytes = true;
        } else {
            stats.byte_count += byte_count;
        }

        stats.flow_count++;
    }
    if (unknown_packets) {
        stats.packet_count = UINT64_MAX;
    }
    if (unknown_bytes) {
        stats.byte_count = UINT64_MAX;
    }

    rule_collection_unref(&rules);
    rule_collection_destroy(&rules);

    reply = ofputil_encode_aggregate_stats_reply(&stats, oh);
    ofconn_send_reply(ofconn, reply);

    return 0;
}
