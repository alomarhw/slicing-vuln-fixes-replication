bool ExtensionPrefs::ReadExtensionPrefURLPatternSet(
    const std::string& extension_id,
    const std::string& pref_key,
    URLPatternSet* result,
    int valid_schemes) {
  const ListValue* value = NULL;
  if (!ReadExtensionPrefList(extension_id, pref_key, &value))
    return false;

  result->ClearPatterns();
  bool allow_file_access = AllowFileAccess(extension_id);

  for (size_t i = 0; i < value->GetSize(); ++i) {
    std::string item;
    if (!value->GetString(i, &item))
      return false;
    URLPattern pattern(valid_schemes);
    if (pattern.Parse(item) != URLPattern::PARSE_SUCCESS) {
      LOG(ERROR) << "Invaid permission pattern: " << item;
      return false;
    }
    if (!allow_file_access && pattern.MatchesScheme(chrome::kFileScheme)) {
      pattern.SetValidSchemes(
          pattern.valid_schemes() & ~URLPattern::SCHEME_FILE);
    }
    result->AddPattern(pattern);
  }

  return true;
}
