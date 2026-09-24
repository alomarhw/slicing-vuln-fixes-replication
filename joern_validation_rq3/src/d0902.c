bool IsInBrowserThumbnailingEnabled() {
#if defined(OS_LINUX) || defined(OS_CHROMEOS)
  return true;
#else
  return CommandLine::ForCurrentProcess()->HasSwitch(
      kEnableInBrowserThumbnailing);
#endif
}
