void SessionService::WindowClosed(const SessionID& window_id) {
  if (!ShouldTrackChangesToWindow(window_id))
    return;

  windows_tracking_.erase(window_id.id());

  if (window_closing_ids_.find(window_id.id()) != window_closing_ids_.end()) {
    window_closing_ids_.erase(window_id.id());
    ScheduleCommand(CreateWindowClosedCommand(window_id.id()));
  } else if (pending_window_close_ids_.find(window_id.id()) ==
             pending_window_close_ids_.end()) {
    has_open_trackable_browsers_ = HasOpenTrackableBrowsers(window_id);
    if (should_record_close_as_pending())
      pending_window_close_ids_.insert(window_id.id());
    else
      ScheduleCommand(CreateWindowClosedCommand(window_id.id()));
  }
}
