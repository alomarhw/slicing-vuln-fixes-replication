void ResourceDispatcherHostImpl::OnAuthRequired(
    net::URLRequest* request,
    net::AuthChallengeInfo* auth_info) {
  if (request->load_flags() & net::LOAD_DO_NOT_PROMPT_FOR_LOGIN) {
    request->CancelAuth();
    return;
  }

  if (delegate_ && !delegate_->AcceptAuthRequest(request, auth_info)) {
    request->CancelAuth();
    return;
  }

  if (!auth_info->is_proxy) {
    HttpAuthResourceType resource_type = HttpAuthResourceTypeOf(request);
    UMA_HISTOGRAM_ENUMERATION("Net.HttpAuthResource",
                              resource_type,
                              HTTP_AUTH_RESOURCE_LAST);

    if (resource_type == HTTP_AUTH_RESOURCE_BLOCKED_CROSS) {
      request->CancelAuth();
      return;
    }
  }


  ResourceRequestInfoImpl* info = ResourceRequestInfoImpl::ForRequest(request);
  DCHECK(!info->login_delegate()) <<
      "OnAuthRequired called with login_delegate pending";
  if (delegate_) {
    info->set_login_delegate(delegate_->CreateLoginDelegate(
        auth_info, request));
  }
  if (!info->login_delegate())
    request->CancelAuth();
}
