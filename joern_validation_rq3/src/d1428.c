ui::TouchStatus UserActivityDetector::PreHandleTouchEvent(
    aura::Window* target,
    aura::TouchEvent* event) {
  MaybeNotify();
  return ui::TOUCH_STATUS_UNKNOWN;
}
