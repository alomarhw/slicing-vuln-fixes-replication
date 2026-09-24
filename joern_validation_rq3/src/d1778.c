void Browser::SelectNextTab() {
  UserMetrics::RecordAction(UserMetricsAction("SelectNextTab"), profile_);
  tab_handler_->GetTabStripModel()->SelectNextTab();
}
