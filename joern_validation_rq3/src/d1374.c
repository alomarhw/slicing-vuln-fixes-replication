  void AddExtensionAndGrantPermissions(const Extension& extension) {
    PermissionsUpdater updater(profile());
    updater.InitializePermissions(&extension);
    updater.GrantActivePermissions(&extension);
    service()->AddExtension(&extension);
  }
