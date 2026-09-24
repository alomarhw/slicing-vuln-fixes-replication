void RenderMediaClient::AddSupportedKeySystems(
    std::vector<std::unique_ptr<media::KeySystemProperties>>*
        key_systems_properties) {
  DVLOG(2) << __func__;
  DCHECK(thread_checker_.CalledOnValidThread());

  GetContentClient()->renderer()->AddSupportedKeySystems(
      key_systems_properties);

  has_updated_ = true;
  last_update_time_ticks_ = tick_clock_->NowTicks();

#if defined(WIDEVINE_CDM_AVAILABLE) && defined(WIDEVINE_CDM_IS_COMPONENT)
  for (const auto& properties : *key_systems_properties) {
    if (properties->GetKeySystemName() == kWidevineKeySystem)
      is_update_needed_ = false;
  }
#else
  is_update_needed_ = false;
#endif
}
