bool ShelfLayoutManager::IsShelfAutoHideForFullscreenMaximized() const {
  wm::WindowState* active_window = wm::GetActiveWindowState();
  return active_window &&
         active_window->autohide_shelf_when_maximized_or_fullscreen();
}
