base::ProcessHandle BrowserRenderProcessHost::GetHandle() {
  if (run_renderer_in_process() || !child_process_launcher_.get())
    return base::Process::Current().handle();

  if (child_process_launcher_->IsStarting())
    return base::kNullProcessHandle;

  return child_process_launcher_->GetHandle();
}
