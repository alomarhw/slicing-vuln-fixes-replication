void RenderViewHostImpl::OnBlur() {
  delegate_->Deactivate();
}
