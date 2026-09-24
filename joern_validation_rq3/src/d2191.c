MessageLoop* MessageLoop::current() {
  return GetTLSMessageLoop()->Get();
}
