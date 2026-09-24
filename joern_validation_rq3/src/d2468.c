BOOL CALLBACK HideWindowsCallback(HWND window, LPARAM param) {
  RenderWidgetHostViewAura* widget =
      reinterpret_cast<RenderWidgetHostViewAura*>(param);
  if (GetProp(window, kWidgetOwnerProperty) == widget)
    SetParent(window, ui::GetHiddenWindow());
  return TRUE;
}
