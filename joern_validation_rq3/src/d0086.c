void AppendMouseWheelEvent(const WebInputEvent& event,
                           std::vector<PP_InputEvent>* pp_events) {
  const WebMouseWheelEvent& mouse_wheel_event =
      reinterpret_cast<const WebMouseWheelEvent&>(event);
  PP_InputEvent result = GetPPEventWithCommonFieldsAndType(event);
  result.u.wheel.modifier = mouse_wheel_event.modifiers;
  result.u.wheel.delta_x = mouse_wheel_event.deltaX;
  result.u.wheel.delta_y = mouse_wheel_event.deltaY;
  result.u.wheel.wheel_ticks_x = mouse_wheel_event.wheelTicksX;
  result.u.wheel.wheel_ticks_y = mouse_wheel_event.wheelTicksY;
  result.u.wheel.scroll_by_page =
      BoolToPPBool(!!mouse_wheel_event.scrollByPage);
  pp_events->push_back(result);
}
