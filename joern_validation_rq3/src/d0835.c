KeyboardEventProcessingResult InterstitialPageImpl::PreHandleKeyboardEvent(
    const NativeWebKeyboardEvent& event) {
  if (!enabled())
    return KeyboardEventProcessingResult::NOT_HANDLED;
  return render_widget_host_delegate_->PreHandleKeyboardEvent(event);
}
