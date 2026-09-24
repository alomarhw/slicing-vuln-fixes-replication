void RunLoop::QuitCurrentDeprecated() {
  DCHECK(IsRunningOnCurrentThread());
  tls_delegate.Get().Get()->active_run_loops_.top()->Quit();
}
