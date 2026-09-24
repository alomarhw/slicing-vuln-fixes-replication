crm_ipc_connect(crm_ipc_t * client)
{
    client->need_reply = FALSE;
    client->ipc = qb_ipcc_connect(client->name, client->buf_size);

    if (client->ipc == NULL) {
        crm_debug("Could not establish %s connection: %s (%d)", client->name, pcmk_strerror(errno), errno);
        return FALSE;
    }

    client->pfd.fd = crm_ipc_get_fd(client);
    if (client->pfd.fd < 0) {
        crm_debug("Could not obtain file descriptor for %s connection: %s (%d)", client->name, pcmk_strerror(errno), errno);
        return FALSE;
    }

    qb_ipcc_context_set(client->ipc, client);

#ifdef HAVE_IPCS_GET_BUFFER_SIZE
    client->max_buf_size = qb_ipcc_get_buffer_size(client->ipc);
    if (client->max_buf_size > client->buf_size) {
        free(client->buffer);
        client->buffer = calloc(1, client->max_buf_size);
        client->buf_size = client->max_buf_size;
    }
#endif

    return TRUE;
}
