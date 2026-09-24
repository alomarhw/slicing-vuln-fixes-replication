unsigned LocalFrameClientImpl::BackForwardLength() {
  WebViewImpl* webview = web_frame_->ViewImpl();
  if (!webview || !webview->Client())
    return 0;
  return webview->Client()->HistoryBackListCount() + 1 +
         webview->Client()->HistoryForwardListCount();
}
