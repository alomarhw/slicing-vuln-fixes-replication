void DevToolsWindow::OpenExternalFrontend(
    Profile* profile,
    const std::string& frontend_url,
    const scoped_refptr<content::DevToolsAgentHost>& agent_host,
    bool is_worker,
    bool is_v8_only) {
  DevToolsWindow* window = FindDevToolsWindow(agent_host.get());
  if (!window) {
    window = Create(profile, GURL(), nullptr, is_worker, is_v8_only,
        DevToolsUI::GetProxyURL(frontend_url).spec(), false, std::string());
    if (!window)
      return;
    window->bindings_->AttachTo(agent_host);
  }

  window->ScheduleShow(DevToolsToggleAction::Show());
}
