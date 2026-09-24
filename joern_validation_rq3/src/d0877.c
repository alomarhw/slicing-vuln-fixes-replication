_handle_error(xmpp_stanza_t *const stanza)
{
    const char *id = xmpp_stanza_get_id(stanza);
    const char *jid = xmpp_stanza_get_from(stanza);
    xmpp_stanza_t *error_stanza = xmpp_stanza_get_child_by_name(stanza, STANZA_NAME_ERROR);
    const char *type = NULL;
    if (error_stanza) {
        type = xmpp_stanza_get_type(error_stanza);
    }

    char *err_msg = stanza_get_error_message(stanza);

    GString *log_msg = g_string_new("message stanza error received");
    if (id) {
        g_string_append(log_msg, " id=");
        g_string_append(log_msg, id);
    }
    if (jid) {
        g_string_append(log_msg, " from=");
        g_string_append(log_msg, jid);
    }
    if (type) {
        g_string_append(log_msg, " type=");
        g_string_append(log_msg, type);
    }
    g_string_append(log_msg, " error=");
    g_string_append(log_msg, err_msg);

    log_info(log_msg->str);

    g_string_free(log_msg, TRUE);

    if (!jid) {
        ui_handle_error(err_msg);
    } else if (type && (strcmp(type, "cancel") == 0)) {
        log_info("Recipient %s not found: %s", jid, err_msg);
        Jid *jidp = jid_create(jid);
        chat_session_remove(jidp->barejid);
        jid_destroy(jidp);
    } else {
        ui_handle_recipient_error(jid, err_msg);
    }

    free(err_msg);
}
