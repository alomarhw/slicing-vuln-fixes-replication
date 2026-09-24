void WebContentsImpl::WasUnOccluded() {
  for (RenderWidgetHostView* view : GetRenderWidgetHostViewsInTree()) {
    if (view)
      view->WasUnOccluded();
  }
}
