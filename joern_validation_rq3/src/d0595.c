bool DownloadManagerImpl::DownloadUrl(
    std::unique_ptr<download::DownloadUrlParameters> params) {
  DownloadUrl(std::move(params), nullptr /* blob_data_handle */,
              nullptr /* blob_url_loader_factory */);
  return true;
}
