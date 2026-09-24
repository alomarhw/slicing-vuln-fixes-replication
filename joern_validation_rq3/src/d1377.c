void GaiaCookieManagerService::AddAccountToCookieWithToken(
    const std::string& account_id,
    const std::string& access_token,
    const std::string& source) {
  VLOG(1) << "GaiaCookieManagerService::AddAccountToCookieWithToken: "
          << account_id;
  DCHECK(!access_token.empty());
  access_token_ = access_token;
  AddAccountToCookieInternal(account_id, source);
}
