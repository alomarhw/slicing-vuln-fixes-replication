  void CloseDevToolsWindow() {
    DevToolsManager* devtools_manager = DevToolsManager::GetInstance();
    content::WindowedNotificationObserver close_observer(
        content::NOTIFICATION_WEB_CONTENTS_DESTROYED,
        content::Source<content::WebContents>(window_->web_contents()));
    devtools_manager->CloseAllClientHosts();
    close_observer.Wait();
  }
