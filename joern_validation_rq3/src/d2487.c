GURL RenderFrameImpl::GetLoadingUrl() const {
  WebDataSource* ds = frame_->dataSource();
  if (ds->hasUnreachableURL())
    return ds->unreachableURL();

  const WebURLRequest& request = ds->request();
  return request.url();
}
