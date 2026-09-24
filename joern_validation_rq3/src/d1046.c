_handle_receipt_received(xmpp_stanza_t *const stanza)
{
    xmpp_stanza_t *receipt = xmpp_stanza_get_child_by_ns(stanza, STANZA_NS_RECEIPTS);
    const char *name = xmpp_stanza_get_name(receipt);
    if (g_strcmp0(name, "received") != 0) {
        return;
    }

    const char *id = xmpp_stanza_get_id(receipt);
    if (!id) {
        return;
    }

    const char *fulljid = xmpp_stanza_get_from(stanza);
    if (!fulljid) {
        return;
    }

    Jid *jidp = jid_create(fulljid);
    sv_ev_message_receipt(jidp->barejid, id);
    jid_destroy(jidp);
}
