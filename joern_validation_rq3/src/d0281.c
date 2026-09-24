void RenderView::OpenURL(
    const GURL& url, const GURL& referrer, WebNavigationPolicy policy) {
  Send(new ViewHostMsg_OpenURL(
      routing_id_, url, referrer, NavigationPolicyToDisposition(policy)));
}
