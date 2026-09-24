  void WaitForState(State state) {
    while (state_ != state)
      MessageLoop::current()->RunAllPending();
  }
