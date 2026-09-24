TreeView::NodeDetails* TreeView::GetNodeDetails(TreeModelNode* node) {
  DCHECK(node &&
         node_to_details_map_.find(node) != node_to_details_map_.end());
  return node_to_details_map_[node];
}
