void config_set_string(config_t *config, const char *section, const char *key, const char *value) {
 section_t *sec = section_find(config, section);
 if (!sec) {
    sec = section_new(section);
    list_append(config->sections, sec);
 }

 for (const list_node_t *node = list_begin(sec->entries); node != list_end(sec->entries); node = list_next(node)) {
 entry_t *entry = list_node(node);
 if (!strcmp(entry->key, key)) {
      osi_free(entry->value);
      entry->value = osi_strdup(value);
 return;
 }
 }

 entry_t *entry = entry_new(key, value);
  list_append(sec->entries, entry);
}
