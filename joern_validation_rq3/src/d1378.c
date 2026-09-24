void AppCacheDispatcherHost::OnSetSpawningHostId(
    int host_id, int spawning_host_id) {
  if (appcache_service_.get()) {
    if (!backend_impl_.SetSpawningHostId(host_id, spawning_host_id))
      bad_message::ReceivedBadMessage(this, bad_message::ACDH_SET_SPAWNING);
  }
}
