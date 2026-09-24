  void GetTabRedirects() {
    DCHECK_CURRENTLY_ON(BrowserThread::UI);
    if (!service_)
      return;

    if (!tab_url_.is_valid()) {
      SendRequest();
      return;
    }

    Profile* profile = Profile::FromBrowserContext(item_->GetBrowserContext());
    history::HistoryService* history = HistoryServiceFactory::GetForProfile(
        profile, ServiceAccessType::EXPLICIT_ACCESS);
    if (!history) {
      SendRequest();
      return;
    }

    history->QueryRedirectsTo(
        tab_url_,
        base::Bind(&CheckClientDownloadRequest::OnGotTabRedirects,
                   base::Unretained(this),
                   tab_url_),
        &request_tracker_);
  }
