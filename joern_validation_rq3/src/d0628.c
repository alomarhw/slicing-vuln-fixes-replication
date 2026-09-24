void OmniboxEditModel::AdjustTextForCopy(int sel_min,
                                         bool is_all_selected,
                                         base::string16* text,
                                         GURL* url,
                                         bool* write_url) {
  *write_url = false;

  if ((sel_min != 0) || controller_->GetToolbarModel()->WouldReplaceURL())
    return;

  if (!user_input_in_progress_ && is_all_selected) {
    *url = PermanentURL();
    *text = base::UTF8ToUTF16(url->spec());
    *write_url = true;
    return;
  }

  AutocompleteMatch match;
  AutocompleteClassifierFactory::GetForProfile(profile_)->Classify(
      *text, KeywordIsSelected(), true, ClassifyPage(), &match, NULL);
  if (AutocompleteMatch::IsSearchType(match.type))
    return;
  *url = match.destination_url;

  GURL perm_url(PermanentURL());
  if (perm_url.SchemeIs(url::kHttpScheme) &&
      url->SchemeIs(url::kHttpScheme) && perm_url.host() == url->host()) {
    *write_url = true;
    base::string16 http = base::ASCIIToUTF16(url::kHttpScheme) +
        base::ASCIIToUTF16(content::kStandardSchemeSeparator);
    if (text->compare(0, http.length(), http) != 0)
      *text = http + *text;
  }
}
