NavigationRequest::~NavigationRequest() {
  TRACE_EVENT_ASYNC_END0("navigation", "NavigationRequest", this);
  if (state_ == STARTED) {
    RenderFrameDevToolsAgentHost::OnNavigationRequestFailed(*this,
                                                            net::ERR_ABORTED);
  }
}
