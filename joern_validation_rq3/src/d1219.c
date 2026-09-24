  GetLRUOriginTask(
      QuotaManager* manager,
      StorageType type,
      const std::map<GURL, int>& origins_in_use,
      const std::map<GURL, int>& origins_in_error,
      GetLRUOriginCallback *callback)
      : DatabaseTaskBase(manager),
        type_(type),
        callback_(callback),
        special_storage_policy_(manager->special_storage_policy_) {
    for (std::map<GURL, int>::const_iterator p = origins_in_use.begin();
         p != origins_in_use.end();
         ++p) {
      if (p->second > 0)
        exceptions_.insert(p->first);
    }
    for (std::map<GURL, int>::const_iterator p = origins_in_error.begin();
         p != origins_in_error.end();
         ++p) {
      if (p->second > QuotaManager::kThresholdOfErrorsToBeBlacklisted)
        exceptions_.insert(p->first);
    }
  }
