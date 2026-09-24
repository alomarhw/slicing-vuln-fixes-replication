void BrowserGpuChannelHostFactory::Terminate() {
  delete instance_;
  instance_ = NULL;
}
