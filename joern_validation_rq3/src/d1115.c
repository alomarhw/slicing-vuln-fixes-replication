bool EnableInProcessStackDumping() {
  struct sigaction sigpipe_action;
  memset(&sigpipe_action, 0, sizeof(sigpipe_action));
  sigpipe_action.sa_handler = SIG_IGN;
  sigemptyset(&sigpipe_action.sa_mask);
  bool success = (sigaction(SIGPIPE, &sigpipe_action, NULL) == 0);

  WarmUpBacktrace();

  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_flags = SA_RESETHAND | SA_SIGINFO;
  action.sa_sigaction = &StackDumpSignalHandler;
  sigemptyset(&action.sa_mask);

  success &= (sigaction(SIGILL, &action, NULL) == 0);
  success &= (sigaction(SIGABRT, &action, NULL) == 0);
  success &= (sigaction(SIGFPE, &action, NULL) == 0);
  success &= (sigaction(SIGBUS, &action, NULL) == 0);
  success &= (sigaction(SIGSEGV, &action, NULL) == 0);
#if !defined(OS_LINUX)
  success &= (sigaction(SIGSYS, &action, NULL) == 0);
#endif  // !defined(OS_LINUX)

  return success;
}
