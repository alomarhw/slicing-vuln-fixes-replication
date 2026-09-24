bool InputDispatcher::isAppSwitchPendingLocked() {
 return mAppSwitchDueTime != LONG_LONG_MAX;
}
