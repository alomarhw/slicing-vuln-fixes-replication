InputMethodTSF::InputMethodTSF(internal::InputMethodDelegate* delegate,
                               HWND toplevel_window_handle)
    : InputMethodWin(delegate, toplevel_window_handle),
      tsf_event_observer_(new TSFEventObserver()),
      tsf_event_router_(new TSFEventRouter(tsf_event_observer_.get())) {
  InputMethodWin::OnFocus();
}
