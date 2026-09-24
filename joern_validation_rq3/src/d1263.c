bool ChromotingInstance::IsConnected() {
  return client_ &&
         (client_->connection_state() == protocol::ConnectionToHost::CONNECTED);
}
