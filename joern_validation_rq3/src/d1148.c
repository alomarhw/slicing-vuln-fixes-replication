IntRect PaintLayerScrollableArea::ScrollCornerRect() const {
  bool has_horizontal_bar = HorizontalScrollbar();
  bool has_vertical_bar = VerticalScrollbar();
  bool has_resizer = GetLayoutBox()->StyleRef().Resize() != EResize::kNone;
  if ((has_horizontal_bar && has_vertical_bar) ||
      (has_resizer && (has_horizontal_bar || has_vertical_bar))) {
    return CornerRect(GetLayoutBox()->PixelSnappedBorderBoxRect(
        Layer()->SubpixelAccumulation()));
  }
  return IntRect();
}
