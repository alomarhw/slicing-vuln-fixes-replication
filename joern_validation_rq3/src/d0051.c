Response InspectorNetworkAgent::setExtraHTTPHeaders(
    const std::unique_ptr<protocol::Network::Headers> headers) {
  state_->setObject(NetworkAgentState::kExtraRequestHeaders,
                    headers->toValue());
  return Response::OK();
}
