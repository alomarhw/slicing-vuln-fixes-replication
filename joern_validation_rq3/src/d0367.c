void PanelBrowserView::OnWorkAreaChanged() {
  BrowserView::OnWorkAreaChanged();
  panel_->manager()->display_settings_provider()->OnDisplaySettingsChanged();
}
