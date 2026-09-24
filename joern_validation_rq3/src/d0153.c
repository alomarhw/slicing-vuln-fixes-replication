unlock_encryption_data_new (DBusGMethodInvocation *context,
                            Device *device,
                            UnlockEncryptionHookFunc hook_func,
                            gpointer hook_user_data)
{
  UnlockEncryptionData *data;

  data = g_new0 (UnlockEncryptionData, 1);
  data->refcount = 1;

  data->context = context;
  data->device = g_object_ref (device);
  data->hook_func = hook_func;
  data->hook_user_data = hook_user_data;
  return data;
}
