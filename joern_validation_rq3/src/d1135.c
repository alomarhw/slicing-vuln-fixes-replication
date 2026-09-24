void BrowserView::LayoutStatusBubble() {
  int overlap = StatusBubbleViews::kShadowThickness;
#if !defined(USE_ASH)
  overlap +=
      IsMaximized() ? 0 : views::NonClientFrameView::kClientEdgeThickness;
#endif
  int height = status_bubble_->GetPreferredSize().height();
  int contents_height = status_bubble_->base_view()->bounds().height();
  gfx::Point origin(-overlap, contents_height - height + overlap);
  status_bubble_->SetBounds(origin.x(), origin.y(), width() / 3, height);
}
