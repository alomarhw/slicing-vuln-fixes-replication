void RenderFrameImpl::SendFailedProvisionalLoad(
    const blink::WebURLRequest& request,
    const WebURLError& error,
    blink::WebLocalFrame* frame) {
  bool show_repost_interstitial =
      (error.reason() == net::ERR_CACHE_MISS &&
       base::EqualsASCII(request.HttpMethod().Utf16(), "POST"));

  FrameHostMsg_DidFailProvisionalLoadWithError_Params params;
  params.error_code = error.reason();
  GetContentClient()->renderer()->PrepareErrorPage(
      this, request, error, nullptr, &params.error_description);
  params.url = error.url(),
  params.showing_repost_interstitial = show_repost_interstitial;
  Send(new FrameHostMsg_DidFailProvisionalLoadWithError(routing_id_, params));
}
