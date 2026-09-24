void MarkKeyboardLoadStarted() {
  if (!g_keyboard_load_time_start.Get().ToInternalValue())
    g_keyboard_load_time_start.Get() = base::Time::Now();
}
