bool PaintLayerScrollableArea::ShouldScrollOnMainThread() const {
  if (HasBeenDisposed())
    return true;
  if (LocalFrame* frame = GetLayoutBox()->GetFrame()) {
    if (frame->View()->GetMainThreadScrollingReasons())
      return true;
  }
  if (HasNonCompositedStickyDescendants())
    return true;
  return ScrollableArea::ShouldScrollOnMainThread();
}
