net::Error ResourceDispatcherHostImpl::BeginDownload(
    scoped_ptr<net::URLRequest> request,
    bool is_content_initiated,
    ResourceContext* context,
    int child_id,
    int route_id,
    bool prefer_cache,
    scoped_ptr<DownloadSaveInfo> save_info,
    content::DownloadId download_id,
    const DownloadStartedCallback& started_callback) {
  if (is_shutdown_)
    return CallbackAndReturn(started_callback, net::ERR_INSUFFICIENT_RESOURCES);

  const GURL& url = request->original_url();

  char url_buf[128];
  base::strlcpy(url_buf, url.spec().c_str(), arraysize(url_buf));
  base::debug::Alias(url_buf);
  CHECK(ContainsKey(active_resource_contexts_, context));

  request->set_referrer(MaybeStripReferrer(GURL(request->referrer())).spec());
  int extra_load_flags = net::LOAD_IS_DOWNLOAD;
  if (prefer_cache) {
    if (request->get_upload() != NULL)
      extra_load_flags |= net::LOAD_ONLY_FROM_CACHE;
    else
      extra_load_flags |= net::LOAD_PREFERRING_CACHE;
  } else {
    extra_load_flags |= net::LOAD_DISABLE_CACHE;
  }
  request->set_load_flags(request->load_flags() | extra_load_flags);
  if (!ChildProcessSecurityPolicyImpl::GetInstance()->
          CanRequestURL(child_id, url)) {
    VLOG(1) << "Denied unauthorized download request for "
            << url.possibly_invalid_spec();
    return CallbackAndReturn(started_callback, net::ERR_ACCESS_DENIED);
  }

  request_id_--;

  const net::URLRequestContext* request_context = context->GetRequestContext();
  if (!request_context->job_factory()->IsHandledURL(url)) {
    VLOG(1) << "Download request for unsupported protocol: "
            << url.possibly_invalid_spec();
    return CallbackAndReturn(started_callback, net::ERR_ACCESS_DENIED);
  }

  ResourceRequestInfoImpl* extra_info =
      CreateRequestInfo(child_id, route_id, true, context);
  extra_info->AssociateWithRequest(request.get());  // Request takes ownership.

  scoped_ptr<ResourceHandler> handler(
      CreateResourceHandlerForDownload(request.get(), is_content_initiated,
                                       true, download_id, save_info.Pass(),
                                       started_callback));

  BeginRequestInternal(request.Pass(), handler.Pass());

  return net::OK;
}
