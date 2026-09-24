void RootWindow::ShowCursor(bool show) {
  cursor_shown_ = show;
  host_->ShowCursor(show);
}
