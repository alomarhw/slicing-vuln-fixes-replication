std::string SyncerProtoUtil::ClientToServerResponseDebugString(
    const sync_pb::ClientToServerResponse& response) {
  std::string output;
  if (response.has_get_updates())
    output.append(GetUpdatesResponseString(response.get_updates()));
  return output;
}
