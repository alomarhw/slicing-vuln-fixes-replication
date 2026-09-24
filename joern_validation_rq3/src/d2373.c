void SyncBackendHost::Core::DoStartSyncing() {
  DCHECK(MessageLoop::current() == host_->core_thread_.message_loop());
  syncapi_->StartSyncingNormally();
  if (deferred_nudge_for_cleanup_requested_)
    syncapi_->RequestNudge(FROM_HERE);
  deferred_nudge_for_cleanup_requested_ = false;
}
