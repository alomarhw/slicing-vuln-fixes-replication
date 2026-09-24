void ResourcePrefetchPredictor::DeleteAllUrls() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  if (initialization_state_ != INITIALIZED) {
    delete_all_data_requested_ = true;
    return;
  }

  host_redirect_data_->DeleteAllData();
  origin_data_->DeleteAllData();
}
