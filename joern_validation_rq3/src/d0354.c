bool BrowserInit::WasRestarted() {
  static bool was_restarted = false;

  static bool was_restarted_read = false;

  if (!was_restarted_read) {
    PrefService* pref_service = g_browser_process->local_state();
    was_restarted = pref_service->GetBoolean(prefs::kWasRestarted);
    pref_service->SetBoolean(prefs::kWasRestarted, false);
    was_restarted_read = true;
  }
  return was_restarted;
}
