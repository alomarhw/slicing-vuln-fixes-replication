bool ShouldCreateDevToolsForNode(FrameTreeNode* ftn) {
  return !ftn->parent() || ftn->current_frame_host()->IsCrossProcessSubframe();
}
