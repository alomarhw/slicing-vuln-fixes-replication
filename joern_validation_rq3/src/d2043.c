void Document::UpdateSecurityOrigin(scoped_refptr<SecurityOrigin> origin) {
  SetSecurityOrigin(std::move(origin));
  DidUpdateSecurityOrigin();
}
