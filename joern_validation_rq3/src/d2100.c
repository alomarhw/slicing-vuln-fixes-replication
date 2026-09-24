void LockContentsView::DoLayout() {
  bool landscape = login_layout_util::ShouldShowLandscape(GetWidget());
  for (auto& action : rotation_actions_)
    action.Run(landscape);

  const display::Display& display =
      display::Screen::GetScreen()->GetDisplayNearestWindow(
          GetWidget()->GetNativeWindow());
  SetPreferredSize(display.size());
  SizeToPreferredSize();
  Layout();
}
