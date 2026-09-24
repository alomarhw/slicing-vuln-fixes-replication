bool SessionService::ShouldNewWindowStartSession() {
  if (!has_open_trackable_browsers_ &&
      !BrowserInit::InSynchronousProfileLaunch() &&
      !SessionRestore::IsRestoring(profile())
#if defined(OS_MACOSX)
      && !app_controller_mac::IsOpeningNewWindow()
#endif
      ) {
    return true;
  }

  return false;
}
