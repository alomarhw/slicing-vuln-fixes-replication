void RenderWidgetHostViewAndroid::WasShown() {
  if (!host_->is_hidden())
    return;

  host_->WasShown();
}
