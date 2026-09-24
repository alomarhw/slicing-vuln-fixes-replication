static bool IsFrameElement(const Node* n) {
  if (!n)
    return false;
  LayoutObject* layout_object = n->GetLayoutObject();
  if (!layout_object || !layout_object->IsLayoutEmbeddedContent())
    return false;
  return ToLayoutEmbeddedContent(layout_object)->ChildFrameView();
}
