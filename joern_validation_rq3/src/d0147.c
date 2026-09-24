ofputil_decode_bundle_add(const struct ofp_header *oh,
                          struct ofputil_bundle_add_msg *msg,
                          enum ofptype *typep)
{
    struct ofpbuf b = ofpbuf_const_initializer(oh, ntohs(oh->length));

    /* Pull the outer ofp_header. */
    enum ofpraw raw = ofpraw_pull_assert(&b);
    ovs_assert(raw == OFPRAW_OFPT14_BUNDLE_ADD_MESSAGE
               || raw == OFPRAW_ONFT13_BUNDLE_ADD_MESSAGE);

    /* Pull the bundle_ctrl header. */
    const struct ofp14_bundle_ctrl_msg *m = ofpbuf_pull(&b, sizeof *m);
    msg->bundle_id = ntohl(m->bundle_id);
    msg->flags = ntohs(m->flags);

    /* Pull the inner ofp_header. */
    if (b.size < sizeof(struct ofp_header)) {
        return OFPERR_OFPBFC_MSG_BAD_LEN;
    }
    msg->msg = b.data;
    if (msg->msg->version != oh->version) {
        return OFPERR_OFPBFC_BAD_VERSION;
    }
    size_t inner_len = ntohs(msg->msg->length);
    if (inner_len < sizeof(struct ofp_header) || inner_len > b.size) {
        return OFPERR_OFPBFC_MSG_BAD_LEN;
    }
    if (msg->msg->xid != oh->xid) {
        return OFPERR_OFPBFC_MSG_BAD_XID;
    }

    /* Reject unbundlable messages. */
    enum ofptype type;
    enum ofperr error = ofptype_decode(&type, msg->msg);
    if (error) {
        VLOG_WARN_RL(&bad_ofmsg_rl, "OFPT14_BUNDLE_ADD_MESSAGE contained "
                     "message is unparsable (%s)", ofperr_get_name(error));
        return OFPERR_OFPBFC_MSG_UNSUP; /* 'error' would be confusing. */
    }

    if (!ofputil_is_bundlable(type)) {
        VLOG_WARN_RL(&bad_ofmsg_rl, "%s message not allowed inside "
                     "OFPT14_BUNDLE_ADD_MESSAGE", ofptype_get_name(type));
        return OFPERR_OFPBFC_MSG_UNSUP;
    }
    if (typep) {
        *typep = type;
    }

    return 0;
}
