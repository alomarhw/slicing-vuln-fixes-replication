int RenderFrameHostManager::CreateRenderFrameProxy(SiteInstance* instance) {
  CHECK(instance);
  CHECK_NE(instance, render_frame_host_->GetSiteInstance());

  RenderViewHostImpl* render_view_host = nullptr;

  render_view_host =
      frame_tree_node_->frame_tree()->GetRenderViewHost(instance);
  if (!render_view_host) {
    CHECK(frame_tree_node_->IsMainFrame());
    render_view_host = frame_tree_node_->frame_tree()->CreateRenderViewHost(
        instance, MSG_ROUTING_NONE, MSG_ROUTING_NONE, true, true);
  }

  RenderFrameProxyHost* proxy = GetRenderFrameProxyHost(instance);
  if (proxy && proxy->is_render_frame_proxy_live())
    return proxy->GetRoutingID();

  if (!proxy)
    proxy = CreateRenderFrameProxyHost(instance, render_view_host);

  if (frame_tree_node_->IsMainFrame()) {
    InitRenderView(render_view_host, proxy);
  } else {
    proxy->InitRenderFrameProxy();
  }

  return proxy->GetRoutingID();
}
