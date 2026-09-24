bool WebContentsImpl::ShouldTransferNavigation() {
  if (!delegate_)
    return true;
  return delegate_->ShouldTransferNavigation();
}
