void BrowserView::ShowCreateChromeAppShortcutsDialog(
    Profile* profile,
    const extensions::Extension* app) {
  chrome::ShowCreateChromeAppShortcutsDialog(GetNativeWindow(), profile, app);
}
