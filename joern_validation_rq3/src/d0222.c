void EventReaderLibevdevCros::OnSynReport(void* data,
                                          EventStateRec* evstate,
                                          struct timeval* tv) {
  EventReaderLibevdevCros* reader = static_cast<EventReaderLibevdevCros*>(data);
  if (reader->ignore_events_)
    return;

  reader->delegate_->OnLibEvdevCrosEvent(&reader->evdev_, evstate, *tv);
}
