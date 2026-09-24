void ProfileSyncService::OnSyncManagedPrefChange(bool is_sync_managed) {
  NotifyObservers();
  if (is_sync_managed) {
    DisableForUser();
  } else if (HasSyncSetupCompleted() &&
             IsSyncEnabledAndLoggedIn() &&
             IsSyncTokenAvailable()) {
    StartUp();
  }
}
