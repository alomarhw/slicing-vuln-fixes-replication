  virtual void OnURLFetchComplete(const URLFetcher* source,
                                  const GURL& url,
                                  const net::URLRequestStatus& status,
                                  int response_code,
                                  const net::ResponseCookies& cookies,
                                  const std::string& data) {
    running_ = (status.status() == net::URLRequestStatus::SUCCESS &&
                response_code == 200 && data.find("ok") == 0);
    MessageLoop::current()->Quit();
  }
