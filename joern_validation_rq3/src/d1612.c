void RenderFrameHostImpl::FailedNavigation(
    const CommonNavigationParams& common_params,
    const BeginNavigationParams& begin_params,
    const RequestNavigationParams& request_params,
    bool has_stale_copy_in_cache,
    int error_code) {
  UpdatePermissionsForNavigation(common_params, request_params);

  ResetWaitingState();

  Send(new FrameMsg_FailedNavigation(routing_id_, common_params, request_params,
                                     has_stale_copy_in_cache, error_code));

  RenderFrameDevToolsAgentHost::OnFailedNavigation(
      this, common_params, begin_params, static_cast<net::Error>(error_code));

  is_loading_ = true;
  frame_tree_node_->ResetNavigationRequest(true, true);
}
