void BlinkTestRunner::OnSessionHistory(
    const std::vector<int>& routing_ids,
    const std::vector<std::vector<PageState>>& session_histories,
    const std::vector<unsigned>& current_entry_indexes) {
  routing_ids_ = routing_ids;
  session_histories_ = session_histories;
  current_entry_indexes_ = current_entry_indexes;
  CaptureDump();
}
