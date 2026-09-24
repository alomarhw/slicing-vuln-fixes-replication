RenderFrameHostImpl::~RenderFrameHostImpl() {
  ResetChildren();

  ResetNavigationRequests();

  ClearAllWebUI();

  SetLastCommittedSiteUrl(GURL());

  if (overlay_routing_token_)
    g_token_frame_map.Get().erase(*overlay_routing_token_);

  site_instance_->RemoveObserver(this);

  const bool was_created = render_frame_created_;
  render_frame_created_ = false;
  if (delegate_ && was_created)
    delegate_->RenderFrameDeleted(this);

  OnAudibleStateChanged(false);

  GetSiteInstance()->DecrementActiveFrameCount();

  if (was_created && render_view_host_->GetMainFrame() != this)
    CHECK(!is_active());

  GetProcess()->RemoveRoute(routing_id_);
  g_routing_id_frame_map.Get().erase(
      RenderFrameHostID(GetProcess()->GetID(), routing_id_));

  swapout_event_monitor_timeout_.reset();

  for (auto& iter : visual_state_callbacks_)
    std::move(iter.second).Run(false);

  if (render_widget_host_ &&
      render_widget_host_->owned_by_render_frame_host()) {
    render_widget_host_->ShutdownAndDestroyWidget(true);
  }

  frame_tree_->ReleaseRenderViewHostRef(render_view_host_);

  RenderFrameHostImpl* beforeunload_initiator = GetBeforeUnloadInitiator();
  if (beforeunload_initiator && beforeunload_initiator != this) {
    base::TimeTicks approx_renderer_start_time = send_before_unload_start_time_;
    beforeunload_initiator->ProcessBeforeUnloadACKFromFrame(
        true /* proceed */, false /* treat_as_final_ack */, this,
        true /* is_frame_being_destroyed */, approx_renderer_start_time,
        base::TimeTicks::Now());
  }
}
