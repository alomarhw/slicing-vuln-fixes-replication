  void UninstallExtension(const std::string& id, bool expect_success) {
    if (expect_success) {
      extensions::TestExtensionRegistryObserver observer(
          extensions::ExtensionRegistry::Get(browser()->profile()));
      extension_service()->UninstallExtension(
          id, extensions::UNINSTALL_REASON_FOR_TESTING, NULL);
      observer.WaitForExtensionUninstalled();
    } else {
      content::WindowedNotificationObserver observer(
          extensions::NOTIFICATION_EXTENSION_UNINSTALL_NOT_ALLOWED,
          content::NotificationService::AllSources());
      extension_service()->UninstallExtension(
          id,
          extensions::UNINSTALL_REASON_FOR_TESTING,
          NULL);
      observer.Wait();
    }
  }
