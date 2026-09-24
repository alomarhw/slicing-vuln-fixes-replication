void WebContentsImpl::WebContentsTreeNode::ConnectToOuterWebContents(
    WebContentsImpl* outer_web_contents,
    RenderFrameHostImpl* outer_contents_frame) {
  focused_web_contents_ = nullptr;
  outer_web_contents_ = outer_web_contents;
  outer_contents_frame_tree_node_id_ =
      outer_contents_frame->frame_tree_node()->frame_tree_node_id();

  outer_web_contents_->node_.AttachInnerWebContents(current_web_contents_);
  outer_contents_frame->frame_tree_node()->AddObserver(this);
}
