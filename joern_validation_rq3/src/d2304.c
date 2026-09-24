void GaiaCookieManagerService::OnLogOutSuccess() {
  DCHECK(requests_.front().request_type() == GaiaCookieRequestType::LOG_OUT);
  VLOG(1) << "GaiaCookieManagerService::OnLogOutSuccess";

  list_accounts_stale_ = true;
  fetcher_backoff_.InformOfRequest(true);
  for (auto& observer : observer_list_) {
    observer.OnLogOutAccountsFromCookieCompleted(
        GoogleServiceAuthError(GoogleServiceAuthError::NONE));
  }
  HandleNextRequest();
}
