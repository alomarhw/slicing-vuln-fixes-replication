  void NotifyNetworkManagerChanged() {
    FOR_EACH_OBSERVER(NetworkManagerObserver,
                      network_manager_observers_,
                      OnNetworkManagerChanged(this));
  }
