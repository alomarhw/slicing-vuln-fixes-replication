void Document::CancelPendingJavaScriptUrl() {
  if (javascript_url_task_handle_.IsActive())
    javascript_url_task_handle_.Cancel();
}
