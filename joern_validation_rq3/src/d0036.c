void Dispatcher::OnActivateExtension(const std::string& extension_id) {
  const Extension* extension =
      RendererExtensionRegistry::Get()->GetByID(extension_id);
  if (!extension) {
    std::string& error = extension_load_errors_[extension_id];
    char minidump[256];
    base::debug::Alias(&minidump);
    base::snprintf(minidump,
                   arraysize(minidump),
                   "e::dispatcher:%s:%s",
                   extension_id.c_str(),
                   error.c_str());
    LOG(FATAL) << extension_id << " was never loaded: " << error;
  }

  active_extension_ids_.insert(extension_id);

  RenderThread::Get()->ScheduleIdleHandler(kInitialExtensionIdleHandlerDelayMs);

  DOMActivityLogger::AttachToWorld(
      DOMActivityLogger::kMainWorldId, extension_id);

  InitOriginPermissions(extension);

  UpdateActiveExtensions();
}
