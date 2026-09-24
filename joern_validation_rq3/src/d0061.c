void PaintLayerScrollableArea::SetVerticalScrollbarVisualRect(
    const LayoutRect& rect) {
  vertical_scrollbar_visual_rect_ = rect;
  if (Scrollbar* scrollbar = VerticalScrollbar())
    scrollbar->SetVisualRect(rect);
}
