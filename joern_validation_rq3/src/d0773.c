int WebSocketExperimentTask::DoWebSocketCloseComplete(int result) {
  websocket_ = NULL;
  return result;
}
