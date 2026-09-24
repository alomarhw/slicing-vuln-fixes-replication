void WebPluginDelegateProxy::OnURLRedirectResponse(bool allow,
                                                   int resource_id) {
  if (!plugin_)
    return;

  plugin_->URLRedirectResponse(allow, resource_id);
}
