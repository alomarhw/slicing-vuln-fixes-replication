void ShellDelegateImpl::Exit() {
  base::MessageLoop::current()->QuitWhenIdle();
}
