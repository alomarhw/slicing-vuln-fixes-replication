partition_create_data_unref (CreatePartitionData *data)
{
  data->refcount--;
  if (data->refcount == 0)
    {
      g_object_unref (data->device);
      g_free (data->fstype);
      g_strfreev (data->fsoptions);
      g_free (data);
    }
}
