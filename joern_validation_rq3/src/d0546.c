int PropertyTreeManager::EnsureCompositorScrollNode(
    const TransformPaintPropertyNode* scroll_offset_translation) {
  const auto* scroll_node = scroll_offset_translation->ScrollNode();
  DCHECK(scroll_node);
  EnsureCompositorTransformNode(scroll_offset_translation);
  auto it = scroll_node_map_.find(scroll_node);
  DCHECK(it != scroll_node_map_.end());
  return it->value;
}
