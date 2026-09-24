void AppCacheUpdateJob::HandleManifestFetchCompleted(
    URLFetcher* fetcher) {
  DCHECK_EQ(internal_state_, FETCH_MANIFEST);
  DCHECK_EQ(manifest_fetcher_, fetcher);
  manifest_fetcher_ = NULL;

  net::URLRequest* request = fetcher->request();
  int response_code = -1;
  bool is_valid_response_code = false;
  if (request->status().is_success()) {
    response_code = request->GetResponseCode();
    is_valid_response_code = (response_code / 100 == 2);

    std::string mime_type;
    request->GetMimeType(&mime_type);
    manifest_has_valid_mime_type_ = (mime_type == "text/cache-manifest");
  }

  if (is_valid_response_code) {
    manifest_data_ = fetcher->manifest_data();
    manifest_response_info_.reset(
        new net::HttpResponseInfo(request->response_info()));
    if (update_type_ == UPGRADE_ATTEMPT)
      CheckIfManifestChanged();  // continues asynchronously
    else
      ContinueHandleManifestFetchCompleted(true);
  } else if (response_code == 304 && update_type_ == UPGRADE_ATTEMPT) {
    ContinueHandleManifestFetchCompleted(false);
  } else if ((response_code == 404 || response_code == 410) &&
             update_type_ == UPGRADE_ATTEMPT) {
    storage_->MakeGroupObsolete(group_, this, response_code);  // async
  } else {
    const char* kFormatString = "Manifest fetch failed (%d) %s";
    std::string message = FormatUrlErrorMessage(
        kFormatString, manifest_url_, fetcher->result(), response_code);
    HandleCacheFailure(AppCacheErrorDetails(message,
                                    APPCACHE_MANIFEST_ERROR,
                                    manifest_url_,
                                    response_code,
                                    false /*is_cross_origin*/),
                       fetcher->result(),
                       GURL());
  }
}
