DictionaryValue* SandboxedExtensionUnpacker::RewriteManifestFile(
    const DictionaryValue& manifest) {
  scoped_ptr<DictionaryValue> final_manifest(
      static_cast<DictionaryValue*>(manifest.DeepCopy()));
  final_manifest->SetString(extension_manifest_keys::kPublicKey, public_key_);

  std::string manifest_json;
  JSONStringValueSerializer serializer(&manifest_json);
  serializer.set_pretty_print(true);
  if (!serializer.Serialize(*final_manifest)) {
    ReportFailure("Error serializing manifest.json.");
    return NULL;
  }

  FilePath manifest_path =
      extension_root_.Append(Extension::kManifestFilename);
  if (!file_util::WriteFile(manifest_path,
                            manifest_json.data(), manifest_json.size())) {
    ReportFailure("Error saving manifest.json.");
    return NULL;
  }

  return final_manifest.release();
}
