void QuotaManager::NotifyStorageAccessedInternal(
    QuotaClient::ID client_id,
    const GURL& origin, StorageType type,
    base::Time accessed_time) {
  LazyInitialize();
  if (type == kStorageTypeTemporary && lru_origin_callback_.get()) {
    access_notified_origins_.insert(origin);
  }

  if (db_disabled_)
    return;
  make_scoped_refptr(new UpdateAccessTimeTask(
      this, origin, type, accessed_time))->Start();
}
