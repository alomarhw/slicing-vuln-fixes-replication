void TemplateURL::InvalidateCachedValues() const {
  for (const TemplateURLRef& ref : url_refs_)
    ref.InvalidateCachedValues();
  suggestions_url_ref_.InvalidateCachedValues();
  instant_url_ref_.InvalidateCachedValues();
  image_url_ref_.InvalidateCachedValues();
  new_tab_url_ref_.InvalidateCachedValues();
  contextual_search_url_ref_.InvalidateCachedValues();
}
