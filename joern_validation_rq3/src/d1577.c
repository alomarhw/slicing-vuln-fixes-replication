void PluginServiceImpl::Init() {
  if (!plugin_list_)
    plugin_list_ = webkit::npapi::PluginList::Singleton();

  plugin_list_token_ = BrowserThread::GetBlockingPool()->GetSequenceToken();
  plugin_list_->set_will_load_plugins_callback(
      base::Bind(&WillLoadPluginsCallback, plugin_list_token_));

  RegisterPepperPlugins();

  const CommandLine* command_line = CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(switches::kSitePerProcess)) {
    webkit::WebPluginInfo webview_plugin(
        ASCIIToUTF16("WebView Tag"),
        FilePath(FILE_PATH_LITERAL("")),
        ASCIIToUTF16("1.2.3.4"),
        ASCIIToUTF16("Browser Plugin."));
    webview_plugin.type = webkit::WebPluginInfo::PLUGIN_TYPE_NPAPI;
    webkit::WebPluginMimeType webview_plugin_mime_type;
    webview_plugin_mime_type.mime_type = "application/browser-plugin";
    webview_plugin_mime_type.file_extensions.push_back("*");
    webview_plugin.mime_types.push_back(webview_plugin_mime_type);
    RegisterInternalPlugin(webview_plugin, true);
  }

  GetContentClient()->AddNPAPIPlugins(plugin_list_);

  FilePath path = command_line->GetSwitchValuePath(switches::kLoadPlugin);
  if (!path.empty())
    AddExtraPluginPath(path);
  path = command_line->GetSwitchValuePath(switches::kExtraPluginDir);
  if (!path.empty())
    plugin_list_->AddExtraPluginDir(path);
}
