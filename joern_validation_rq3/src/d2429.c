void WebContentsImpl::OnMediaNotification(int64 player_cookie,
                                          bool has_video,
                                          bool has_audio,
                                          bool is_playing) {
#if !defined(OS_CHROMEOS)
  if (is_playing) {
    scoped_ptr<PowerSaveBlocker> blocker;
    if (has_video) {
      blocker = PowerSaveBlocker::Create(
          PowerSaveBlocker::kPowerSaveBlockPreventDisplaySleep,
          "Playing video");
#if defined(OS_ANDROID)
      static_cast<PowerSaveBlockerImpl*>(blocker.get())->
          InitDisplaySleepBlocker(GetView()->GetNativeView());
#endif
    } else if (has_audio) {
      blocker = PowerSaveBlocker::Create(
          PowerSaveBlocker::kPowerSaveBlockPreventAppSuspension,
          "Playing audio");
    }

    if (blocker) {
      power_save_blockers_[render_view_message_source_][player_cookie] =
          blocker.release();
    }
  } else {
    delete power_save_blockers_[render_view_message_source_][player_cookie];
    power_save_blockers_[render_view_message_source_].erase(player_cookie);
  }
#endif  // !defined(OS_CHROMEOS)
}
