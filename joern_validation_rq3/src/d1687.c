void CommandBufferProxyImpl::OnEchoAck() {
  DCHECK(!echo_tasks_.empty());
  base::Closure callback = echo_tasks_.front();
  echo_tasks_.pop();
  callback.Run();
}
