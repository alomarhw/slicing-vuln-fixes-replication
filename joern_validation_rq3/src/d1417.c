qreal OxideQQuickWebView::zoomFactor() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return 1.0;
  }

  return d->proxy_->zoomFactor();
}
