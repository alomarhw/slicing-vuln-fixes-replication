static void plugin_instance_deallocate(PluginInstance *plugin)
{
  NPW_MemFree(plugin);
}
