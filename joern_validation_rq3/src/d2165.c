int stacked_tab_right_clip() {
  static int value = -1;
  if (value == -1) {
    switch (ui::GetDisplayLayout()) {
      case ui::LAYOUT_DESKTOP:
        value = 20;
        break;
      case ui::LAYOUT_TOUCH:
        value = 26;
        break;
      default:
        NOTREACHED();
    }
  }
  return value;
}
