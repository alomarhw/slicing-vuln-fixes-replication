HRESULT CGaiaCredentialBase::SaveAccountInfo(
    const base::DictionaryValue& properties) {
  LOGFN(INFO);

  base::string16 sid = GetDictString(&properties, kKeySID);
  if (sid.empty()) {
    LOGFN(ERROR) << "SID is empty";
    return E_INVALIDARG;
  }

  base::string16 username = GetDictString(&properties, kKeyUsername);
  if (username.empty()) {
    LOGFN(ERROR) << "Username is empty";
    return E_INVALIDARG;
  }

  base::string16 password = GetDictString(&properties, kKeyPassword);
  if (password.empty()) {
    LOGFN(ERROR) << "Password is empty";
    return E_INVALIDARG;
  }

  base::string16 domain = GetDictString(&properties, kKeyDomain);

  auto profile = ScopedUserProfile::Create(sid, domain, username, password);
  if (!profile) {
    LOGFN(ERROR) << "Could not load user profile";
    return E_UNEXPECTED;
  }

  HRESULT hr = profile->SaveAccountInfo(properties);
  if (FAILED(hr))
    LOGFN(ERROR) << "profile.SaveAccountInfo failed (cont) hr=" << putHR(hr);

  return hr;
}
