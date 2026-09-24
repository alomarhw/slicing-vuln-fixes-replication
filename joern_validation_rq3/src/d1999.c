void InputDispatcher::doInterceptKeyBeforeDispatchingLockedInterruptible(
 CommandEntry* commandEntry) {
 KeyEntry* entry = commandEntry->keyEntry;

 KeyEvent event;
    initializeKeyEvent(&event, entry);

    mLock.unlock();

 nsecs_t delay = mPolicy->interceptKeyBeforeDispatching(commandEntry->inputWindowHandle,
 &event, entry->policyFlags);

    mLock.lock();

 if (delay < 0) {
        entry->interceptKeyResult = KeyEntry::INTERCEPT_KEY_RESULT_SKIP;
 } else if (!delay) {
        entry->interceptKeyResult = KeyEntry::INTERCEPT_KEY_RESULT_CONTINUE;
 } else {
        entry->interceptKeyResult = KeyEntry::INTERCEPT_KEY_RESULT_TRY_AGAIN_LATER;
        entry->interceptKeyWakeupTime = now() + delay;
 }
    entry->release();
}
