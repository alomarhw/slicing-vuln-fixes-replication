void MasterPreferences::InitializeFromFilePath(
    const base::FilePath& prefs_path) {
  std::string json_data;
  if (base::PathExists(prefs_path) &&
      !base::ReadFileToString(prefs_path, &json_data)) {
    LOG(ERROR) << "Failed to read preferences from " << prefs_path.value();
  }
  if (InitializeFromString(json_data))
    preferences_read_from_file_ = true;
}
