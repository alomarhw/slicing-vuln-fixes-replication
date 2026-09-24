void ScrollableShelfView::ScrollToNewPage(bool forward) {
  float offset = CalculatePageScrollingOffset(forward);
  if (GetShelf()->IsHorizontalAlignment())
    ScrollByXOffset(offset, /*animating=*/true);
  else
    ScrollByYOffset(offset, /*animating=*/true);
}
