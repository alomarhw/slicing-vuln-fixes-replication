ProfileImplIOData::Handle::~Handle() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (main_request_context_getter_)
    main_request_context_getter_->CleanupOnUIThread();
  if (media_request_context_getter_)
    media_request_context_getter_->CleanupOnUIThread();
  if (extensions_request_context_getter_)
    extensions_request_context_getter_->CleanupOnUIThread();

  if (io_data_->predictor_.get() != NULL) {
    PrefService* user_prefs = profile_->GetPrefs();
    io_data_->predictor_->ShutdownOnUIThread(user_prefs);
  }

  for (ChromeURLRequestContextGetterMap::iterator iter =
           app_request_context_getter_map_.begin();
       iter != app_request_context_getter_map_.end();
       ++iter) {
    iter->second->CleanupOnUIThread();
  }

  if (io_data_->http_server_properties_manager_.get())
    io_data_->http_server_properties_manager_->ShutdownOnUIThread();
  io_data_->ShutdownOnUIThread();
}
