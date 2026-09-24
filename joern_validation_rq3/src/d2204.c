bool MetricsLog::LoadSavedEnvironmentFromPrefs(PrefService* local_state,
                                               std::string* app_version) {
  DCHECK(!has_environment_);
  has_environment_ = true;
  app_version->clear();

  SystemProfileProto* system_profile = uma_proto()->mutable_system_profile();
  EnvironmentRecorder recorder(local_state);
  bool success = recorder.LoadEnvironmentFromPrefs(system_profile);
  if (success)
    *app_version = system_profile->app_version();
  return success;
}
