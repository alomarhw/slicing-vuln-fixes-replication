void NavigationControllerImpl::HandleRendererDebugURL(
    FrameTreeNode* frame_tree_node,
    const GURL& url) {
  if (!frame_tree_node->current_frame_host()->IsRenderFrameLive()) {
    if (!IsInitialNavigation()) {
      DiscardNonCommittedEntries();
      return;
    }
    frame_tree_node->render_manager()->InitializeRenderFrameIfNecessary(
        frame_tree_node->current_frame_host());
  }
  frame_tree_node->current_frame_host()->HandleRendererDebugURL(url);
}
