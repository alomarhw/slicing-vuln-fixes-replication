void OnSettingsAppFinished(
    std::unique_ptr<DefaultBrowserActionRecorder> recorder,
    const base::Closure& on_finished_callback) {
  recorder.reset();
  on_finished_callback.Run();
}
