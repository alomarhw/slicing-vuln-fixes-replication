void ExtensionPrefs::SetExtensionPrefPermissionSet(
    const std::string& extension_id,
    const std::string& pref_key,
    const ExtensionPermissionSet* new_value) {
  ListValue* api_values = new ListValue();
  ExtensionAPIPermissionSet apis = new_value->apis();
  ExtensionPermissionsInfo* info = ExtensionPermissionsInfo::GetInstance();
  std::string api_pref = JoinPrefs(pref_key, kPrefAPIs);
  for (ExtensionAPIPermissionSet::const_iterator i = apis.begin();
       i != apis.end(); ++i) {
    ExtensionAPIPermission* perm = info->GetByID(*i);
    if (perm)
      api_values->Append(Value::CreateStringValue(perm->name()));
  }
  UpdateExtensionPref(extension_id, api_pref, api_values);

  if (!new_value->explicit_hosts().is_empty()) {
    SetExtensionPrefURLPatternSet(extension_id,
                                  JoinPrefs(pref_key, kPrefExplicitHosts),
                                  new_value->explicit_hosts());
  }

  if (!new_value->scriptable_hosts().is_empty()) {
    SetExtensionPrefURLPatternSet(extension_id,
                                  JoinPrefs(pref_key, kPrefScriptableHosts),
                                  new_value->scriptable_hosts());
  }
}
