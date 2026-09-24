void ExtensionDevToolsClientHost::OnExtensionUnloaded(
    content::BrowserContext* browser_context,
    const Extension* extension,
    UnloadedExtensionInfo::Reason reason) {
  if (extension->id() == extension_id_)
    Close();
}
