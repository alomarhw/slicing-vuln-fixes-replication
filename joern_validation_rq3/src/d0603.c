void WebContentsImpl::OnDidProgrammaticallyScroll(
    const gfx::Vector2d& scroll_point) {
  if (delegate_)
    delegate_->DidProgrammaticallyScroll(this, scroll_point);
}
