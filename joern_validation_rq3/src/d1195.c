void RenderViewHostImpl::OnDidRedirectProvisionalLoad(
    int32 page_id,
    const GURL& source_url,
    const GURL& target_url) {
  delegate_->DidRedirectProvisionalLoad(
      this, page_id, source_url, target_url);
}
