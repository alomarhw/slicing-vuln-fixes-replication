static void free_server_handle(kadm5_server_handle_t handle)
{
    if (!handle)
        return;
    krb5_free_principal(handle->context, handle->current_caller);
    free(handle);
}
