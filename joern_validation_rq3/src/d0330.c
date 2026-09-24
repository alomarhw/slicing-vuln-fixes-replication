void RenderFrameHostManager::CommitPendingIfNecessary(
    RenderFrameHostImpl* render_frame_host,
    bool was_caused_by_user_gesture) {
  if (!speculative_render_frame_host_) {

    DCHECK_EQ(render_frame_host_.get(), render_frame_host);

    if (render_frame_host_->pending_web_ui())
      CommitPendingWebUI();
    return;
  }

  if (render_frame_host == speculative_render_frame_host_.get()) {
    CommitPending();
    frame_tree_node_->ResetNavigationRequest(false, true);
  } else if (render_frame_host == render_frame_host_.get()) {

    if (render_frame_host_->pending_web_ui())
      CommitPendingWebUI();

    if (was_caused_by_user_gesture) {
      frame_tree_node_->ResetNavigationRequest(false, true);
      CleanUpNavigation();
    }
  } else {
    NOTREACHED();
  }
}
