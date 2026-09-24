int SocketStream::DoGenerateProxyAuthTokenComplete(int result) {
  if (result != OK) {
    next_state_ = STATE_CLOSE;
    return result;
  }

  next_state_ = STATE_WRITE_TUNNEL_HEADERS;
  return result;
}
