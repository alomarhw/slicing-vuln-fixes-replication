AutomationProviderBookmarkModelObserver(
    AutomationProvider* provider,
    IPC::Message* reply_message,
    BookmarkModel* model,
    bool use_json_interface)
    : automation_provider_(provider->AsWeakPtr()),
      reply_message_(reply_message),
      model_(model),
      use_json_interface_(use_json_interface) {
  model_->AddObserver(this);
}
