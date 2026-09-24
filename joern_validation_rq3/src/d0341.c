void WebContentsImpl::OnAudioStateChanged(bool is_audible) {
  SendPageMessage(new PageMsg_AudioStateChanged(MSG_ROUTING_NONE, is_audible));

  NotifyNavigationStateChanged(INVALIDATE_TYPE_TAB);
}
