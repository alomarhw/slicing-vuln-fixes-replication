bool FrameSelection::IsHidden() const {
  if (SelectionHasFocus())
    return false;

  const Node* start =
      ComputeVisibleSelectionInDOMTree().Start().ComputeContainerNode();
  if (!start)
    return true;

  if (!GetSelectionInDOMTree().IsRange())
    return true;

  return EnclosingTextControl(start);
}
