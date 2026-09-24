  void GetAvailableSpace() {
    quota_status_ = kQuotaStatusUnknown;
    available_space_ = -1;
    quota_manager_->GetAvailableSpace(
        callback_factory_.NewCallback(
            &QuotaManagerTest::DidGetAvailableSpace));
  }
