void FindMatchesByUsername(const PasswordFormFillData& fill_data,
                           const base::string16& current_username,
                           bool exact_username_match,
                           RendererSavePasswordProgressLogger* logger,
                           base::string16* username,
                           base::string16* password) {
  if (DoUsernamesMatch(fill_data.username_field.value, current_username,
                       exact_username_match)) {
    *username = fill_data.username_field.value;
    *password = fill_data.password_field.value;
    if (logger)
      logger->LogMessage(Logger::STRING_USERNAMES_MATCH);
  } else {
    for (const auto& it : fill_data.additional_logins) {
      if (!it.second.realm.empty()) {
        continue;
      }
      if (DoUsernamesMatch(it.first, current_username, exact_username_match)) {
        *username = it.first;
        *password = it.second.password;
        break;
      }
    }
    if (logger) {
      logger->LogBoolean(Logger::STRING_MATCH_IN_ADDITIONAL,
                         !(username->empty() && password->empty()));
    }
  }
}
