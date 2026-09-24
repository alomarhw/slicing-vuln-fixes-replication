void TestingAutomationProvider::OnSourceProfilesLoaded() {
  DCHECK_NE(static_cast<ImporterList*>(NULL), importer_list_.get());

  importer::SourceProfile source_profile;
  size_t i = 0;
  size_t importers_count = importer_list_->count();
  for ( ; i < importers_count; ++i) {
    importer::SourceProfile profile = importer_list_->GetSourceProfileAt(i);
    if (profile.importer_name == import_settings_data_.browser_name) {
      source_profile = profile;
      break;
    }
  }
  if (i == importers_count) {
    AutomationJSONReply(this, import_settings_data_.reply_message)
        .SendError("Invalid browser name string found.");
    return;
  }

  scoped_refptr<ImporterHost> importer_host(new ImporterHost);
  importer_host->SetObserver(
      new AutomationProviderImportSettingsObserver(
          this, import_settings_data_.reply_message));

  Profile* target_profile = import_settings_data_.browser->profile();
  importer_host->StartImportSettings(source_profile,
                                     target_profile,
                                     import_settings_data_.import_items,
                                     new ProfileWriter(target_profile),
                                     import_settings_data_.first_run);
}
