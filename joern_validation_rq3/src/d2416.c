void DesktopWindowTreeHostX11::SetAlwaysOnTop(bool always_on_top) {
  is_always_on_top_ = always_on_top;
  SetWMSpecState(always_on_top, gfx::GetAtom("_NET_WM_STATE_ABOVE"), x11::None);
}
