void RTCPeerConnectionHandler::OnRenegotiationNeeded() {
  DCHECK(task_runner_->RunsTasksInCurrentSequence());
  TRACE_EVENT0("webrtc", "RTCPeerConnectionHandler::OnRenegotiationNeeded");
  if (peer_connection_tracker_)
    peer_connection_tracker_->TrackOnRenegotiationNeeded(this);
  if (!is_closed_)
    client_->NegotiationNeeded();
}
