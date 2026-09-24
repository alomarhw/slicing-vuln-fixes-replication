bool BrowserView::IsPositionInWindowCaption(const gfx::Point& point) {
  if (window_switcher_button_) {
    gfx::Point window_switcher_point(point);
    views::View::ConvertPointToTarget(this, window_switcher_button_,
                                      &window_switcher_point);
    if (window_switcher_button_->HitTestPoint(window_switcher_point))
      return false;
  }
  return GetBrowserViewLayout()->IsPositionInWindowCaption(point);
}
