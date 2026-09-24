void WriteFromUrlOperation::OnURLFetchComplete(const net::URLFetcher* source) {
  DCHECK_CURRENTLY_ON(BrowserThread::FILE);

  if (source->GetStatus().is_success() && source->GetResponseCode() == 200) {
    SetProgress(kProgressComplete);

    download_continuation_.Run();

    download_continuation_ = base::Closure();
  } else {
    Error(error::kDownloadInterrupted);
  }
}
