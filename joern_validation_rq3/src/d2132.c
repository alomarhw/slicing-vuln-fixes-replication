void RenderViewImpl::OnWasShown(bool needs_repainting) {
  RenderWidget::OnWasShown(needs_repainting);

  if (webview()) {
    webview()->settings()->setMinimumTimerInterval(
        webkit_glue::kForegroundTabTimerInterval);
    webview()->setVisibilityState(visibilityState(), false);
  }

  pepper_helper_->PageVisibilityChanged(true);

#if defined(OS_MACOSX)
  std::set<WebPluginDelegateProxy*>::iterator plugin_it;
  for (plugin_it = plugin_delegates_.begin();
       plugin_it != plugin_delegates_.end(); ++plugin_it) {
    (*plugin_it)->SetContainerVisibility(true);
  }
#endif  // OS_MACOSX
}
