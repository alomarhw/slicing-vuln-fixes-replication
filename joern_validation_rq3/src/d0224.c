GURL GetEffectiveURLForInstant(const GURL& url, Profile* profile) {
  CHECK(chrome::search::ShouldAssignURLToInstantRenderer(url, profile))
      << "Error granting Instant access.";

  if (url.SchemeIs(chrome::kChromeSearchScheme))
    return url;

  GURL effective_url(url);

  url_canon::Replacements<char> replacements;
  std::string search_scheme(chrome::kChromeSearchScheme);
  replacements.SetScheme(search_scheme.data(),
                         url_parse::Component(0, search_scheme.length()));
  effective_url = effective_url.ReplaceComponents(replacements);
  return effective_url;
}
