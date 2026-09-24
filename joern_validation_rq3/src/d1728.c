RefPtr<SecurityOrigin> FrameFetchContext::GetRequestorOrigin() {
  if (IsDetached())
    return frozen_state_->requestor_origin;

  if (document_->IsSandboxed(kSandboxOrigin))
    return SecurityOrigin::Create(document_->Url());

  return GetSecurityOrigin();
}
