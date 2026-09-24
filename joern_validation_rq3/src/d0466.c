WebMouseWheelEvent* BuildMouseWheelEvent(const PP_InputEvent& event) {
  WebMouseWheelEvent* mouse_wheel_event = new WebMouseWheelEvent();
  mouse_wheel_event->type = WebInputEvent::MouseWheel;
  mouse_wheel_event->timeStampSeconds = event.time_stamp;
  mouse_wheel_event->modifiers = event.u.wheel.modifier;
  mouse_wheel_event->deltaX = event.u.wheel.delta_x;
  mouse_wheel_event->deltaY = event.u.wheel.delta_y;
  mouse_wheel_event->wheelTicksX = event.u.wheel.wheel_ticks_x;
  mouse_wheel_event->wheelTicksY = event.u.wheel.wheel_ticks_y;
  mouse_wheel_event->scrollByPage = event.u.wheel.scroll_by_page;
  return mouse_wheel_event;
}
