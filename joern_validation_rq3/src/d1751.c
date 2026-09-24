void NetworkScreen::NotifyOnConnection() {
  UnsubscribeNetworkNotification();
  connection_timer_.Stop();
  delegate()->GetObserver(this)->OnExit(ScreenObserver::NETWORK_CONNECTED);
}
