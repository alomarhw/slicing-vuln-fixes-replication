  void InvokeResetScreen() {
    ASSERT_TRUE(JSExecuted("cr.ui.Oobe.handleAccelerator('reset');"));
    OobeScreenWaiter(OobeDisplay::SCREEN_OOBE_RESET).Wait();
  }
