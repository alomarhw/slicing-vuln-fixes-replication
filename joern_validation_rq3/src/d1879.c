    OVS_REQUIRES(ofproto_mutex)
{
    const struct rule_actions *actions;
    struct ofputil_flow_update fu;

    fu.event = (flags & (NXFMF_INITIAL | NXFMF_ADD)
                ? NXFME_ADDED : NXFME_MODIFIED);
    fu.reason = 0;
    ovs_mutex_lock(&rule->mutex);
    fu.idle_timeout = rule->idle_timeout;
    fu.hard_timeout = rule->hard_timeout;
    ovs_mutex_unlock(&rule->mutex);
    fu.table_id = rule->table_id;
    fu.cookie = rule->flow_cookie;
    minimatch_expand(&rule->cr.match, &fu.match);
    fu.priority = rule->cr.priority;

    actions = flags & NXFMF_ACTIONS ? rule_get_actions(rule) : NULL;
    fu.ofpacts = actions ? actions->ofpacts : NULL;
    fu.ofpacts_len = actions ? actions->ofpacts_len : 0;

    if (ovs_list_is_empty(msgs)) {
        ofputil_start_flow_update(msgs);
    }
    ofputil_append_flow_update(&fu, msgs, tun_table);
}
