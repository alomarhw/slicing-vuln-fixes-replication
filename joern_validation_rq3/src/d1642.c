gfx::Size BrowserView::GetNTPBackgroundFillSize() const {
  if (!chrome::search::IsInstantExtendedAPIEnabled(browser()->profile()))
    return gfx::Size();
  gfx::Rect content_bounds = contents_container_->bounds();
  gfx::Point origin(content_bounds.origin());
  View::ConvertPointToTarget(contents_container_->parent(), this, &origin);
  content_bounds.set_origin(origin);
  return gfx::Size(content_bounds.right(), content_bounds.bottom());
}
