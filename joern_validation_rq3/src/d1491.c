void SyncManager::SyncInternal::OnServerConnectionEvent(
    const ServerConnectionEvent& event) {
  DCHECK(thread_checker_.CalledOnValidThread());
  allstatus_.HandleServerConnectionEvent(event);
  if (event.connection_code ==
      browser_sync::HttpResponse::SERVER_CONNECTION_OK) {
    FOR_EACH_OBSERVER(SyncManager::Observer, observers_,
                      OnAuthError(AuthError::None()));
  }

  if (event.connection_code == browser_sync::HttpResponse::SYNC_AUTH_ERROR) {
    observing_ip_address_changes_ = false;
    FOR_EACH_OBSERVER(SyncManager::Observer, observers_,
        OnAuthError(AuthError(AuthError::INVALID_GAIA_CREDENTIALS)));
  }

  if (event.connection_code ==
      browser_sync::HttpResponse::SYNC_SERVER_ERROR) {
    FOR_EACH_OBSERVER(SyncManager::Observer, observers_,
        OnAuthError(AuthError(AuthError::CONNECTION_FAILED)));
  }
}
