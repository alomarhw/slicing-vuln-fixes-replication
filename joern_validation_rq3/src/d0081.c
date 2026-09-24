void RenderView::OnExtensionMessageInvoke(const std::string& function_name,
                                          const ListValue& args,
                                          bool requires_incognito_access,
                                          const GURL& event_url) {
  RendererExtensionBindings::Invoke(
      function_name, args, this, requires_incognito_access, event_url);
}
