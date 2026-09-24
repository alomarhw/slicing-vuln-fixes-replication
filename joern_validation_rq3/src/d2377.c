daemon_linux_lvm2_lv_stop_authorized_cb (Daemon *daemon,
                                         Device *device,
                                         DBusGMethodInvocation *context,
                                         const gchar *action_id,
                                         guint num_user_data,
                                         gpointer *user_data_elements)
{
  gchar *name;
  /* TODO: use options */
  guint n;
  gchar *argv[10];

  name = NULL;

  if (!device->priv->device_is_linux_lvm2_lv)
    {
      throw_error (context, ERROR_FAILED, "Device is not a Linux LVM2 Logical Volume");
      goto out;
    }

  /* Unfortunately lvchange does not (yet - file a bug) accept UUIDs
   */
  name = g_strdup_printf ("%s/%s", device->priv->linux_lvm2_lv_group_name, device->priv->linux_lvm2_lv_name);

  n = 0;
  argv[n++] = "lvchange";
  argv[n++] = "-an";
  argv[n++] = name;
  argv[n++] = NULL;

  if (!job_new (context, "LinuxLvm2LVStop", TRUE, NULL, argv, NULL, linux_lvm2_lv_stop_completed_cb, FALSE, NULL, NULL))
    {
      goto out;
    }

 out:
  g_free (name);
}
