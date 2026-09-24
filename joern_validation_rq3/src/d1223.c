std::unique_ptr<TracedValue> InspectorTimerRemoveEvent::Data(
    ExecutionContext* context,
    int timer_id) {
  std::unique_ptr<TracedValue> value = GenericTimerData(context, timer_id);
  SetCallStack(value.get());
  return value;
}
