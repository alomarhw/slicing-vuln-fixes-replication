bool DragController::IsCopyKeyDown(DragData* drag_data) {
  int modifiers = drag_data->GetModifiers();

#if defined(OS_MACOSX)
  return modifiers & WebInputEvent::kAltKey;
#else
  return modifiers & WebInputEvent::kControlKey;
#endif
}
