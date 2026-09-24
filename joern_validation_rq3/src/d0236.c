void HTMLMediaElement::progressEventTimerFired(TimerBase*) {
  if (m_networkState != kNetworkLoading)
    return;

  double time = WTF::currentTime();
  double timedelta = time - m_previousProgressTime;

  if (webMediaPlayer() && webMediaPlayer()->didLoadingProgress()) {
    scheduleEvent(EventTypeNames::progress);
    m_previousProgressTime = time;
    m_sentStalledEvent = false;
    if (layoutObject())
      layoutObject()->updateFromElement();
  } else if (timedelta > 3.0 && !m_sentStalledEvent) {
    scheduleEvent(EventTypeNames::stalled);
    m_sentStalledEvent = true;
    setShouldDelayLoadEvent(false);
  }
}
