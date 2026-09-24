void ProfileSyncService::DisableForUser() {
  sync_prefs_.ClearPreferences();
  ClearUnrecoverableError();
  ShutdownImpl(true);

  if (!auto_start_enabled_)
    signin_->SignOut();

  NotifyObservers();
}
