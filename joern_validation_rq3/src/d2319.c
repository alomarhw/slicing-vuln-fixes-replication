void LockContentsView::OnDisplayMetricsChanged(const display::Display& display,
                                               uint32_t changed_metrics) {
  if ((changed_metrics & DISPLAY_METRIC_ROTATION) == 0)
    return;

  DoLayout();
}
