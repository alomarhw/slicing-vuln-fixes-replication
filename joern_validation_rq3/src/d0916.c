BrowserContextIOData::BrowserContextIOData()
    : resource_context_(new ResourceContext(this)),
      temporary_saved_permission_context_(
        new TemporarySavedPermissionContext()) {}
