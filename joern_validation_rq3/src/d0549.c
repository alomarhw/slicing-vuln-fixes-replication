void RenderWidgetHostImpl::StartHangMonitorTimeout(TimeDelta delay) {
  if (CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kDisableHangMonitor)) {
    return;
  }

  Time requested_end_time = Time::Now() + delay;
  if (time_when_considered_hung_.is_null() ||
      time_when_considered_hung_ > requested_end_time)
    time_when_considered_hung_ = requested_end_time;

  if (hung_renderer_timer_.IsRunning() &&
      hung_renderer_timer_.GetCurrentDelay() <= delay) {
    return;
  }

  time_when_considered_hung_ = requested_end_time;
  hung_renderer_timer_.Stop();
  hung_renderer_timer_.Start(FROM_HERE, delay, this,
      &RenderWidgetHostImpl::CheckRendererIsUnresponsive);
}
