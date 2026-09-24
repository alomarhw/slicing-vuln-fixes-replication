void PresentationConnection::didReceiveTextMessage(const WebString& message) {
  if (m_state != WebPresentationConnectionState::Connected)
    return;

  dispatchEvent(MessageEvent::create(message));
}
