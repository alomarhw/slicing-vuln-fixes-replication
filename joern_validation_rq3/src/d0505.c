WebKit::WebPeerConnectionHandler* MediaStreamImpl::CreatePeerConnectionHandler(
    WebKit::WebPeerConnectionHandlerClient* client) {
  DCHECK(CalledOnValidThread());
  if (peer_connection_handler_) {
    DVLOG(1) << "A PeerConnection already exists";
    return NULL;
  }
  EnsurePeerConnectionFactory();

  peer_connection_handler_ = new PeerConnectionHandler(
      client,
      this,
      dependency_factory_.get());

  return peer_connection_handler_;
}
