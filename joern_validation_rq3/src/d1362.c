ui::TextInputType RenderViewImpl::GetTextInputType() {
  return pepper_delegate_.IsPluginFocused() ?
      pepper_delegate_.GetTextInputType() : RenderWidget::GetTextInputType();
}
