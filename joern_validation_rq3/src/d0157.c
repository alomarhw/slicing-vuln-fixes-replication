void AddGoogleAssistantStrings(content::WebUIDataSource* html_source) {
  LocalizedString localized_strings[] = {
      {"googleAssistantPageTitle", IDS_SETTINGS_GOOGLE_ASSISTANT},
      {"googleAssistantEnableContext",
       IDS_SETTINGS_GOOGLE_ASSISTANT_ENABLE_CONTEXT},
      {"googleAssistantEnableContextDescription",
       IDS_SETTINGS_GOOGLE_ASSISTANT_ENABLE_CONTEXT_DESCRIPTION},
      {"googleAssistantSettings", IDS_SETTINGS_GOOGLE_ASSISTANT_SETTINGS},
  };
  AddLocalizedStringsBulk(html_source, localized_strings,
                          arraysize(localized_strings));
}
