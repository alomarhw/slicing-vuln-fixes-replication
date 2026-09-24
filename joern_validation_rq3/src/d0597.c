status_t Camera3Device::registerInFlight(uint32_t frameNumber,
 int32_t numBuffers, CaptureResultExtras resultExtras, bool hasInput,
 const AeTriggerCancelOverride_t &aeTriggerCancelOverride) {
    ATRACE_CALL();
 Mutex::Autolock l(mInFlightLock);

 ssize_t res;
    res = mInFlightMap.add(frameNumber, InFlightRequest(numBuffers, resultExtras, hasInput,
            aeTriggerCancelOverride));
 if (res < 0) return res;

 return OK;
}
