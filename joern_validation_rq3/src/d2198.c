void RenderViewImpl::OnPpapiBrokerPermissionResult(
    int request_id,
    bool result) {
  pepper_helper_->OnPpapiBrokerPermissionResult(request_id, result);
}
