status_t MediaRecorder::stop()
{
    ALOGV("stop");
 if (mMediaRecorder == NULL) {
        ALOGE("media recorder is not initialized yet");
 return INVALID_OPERATION;
 }
 if (!(mCurrentState & MEDIA_RECORDER_RECORDING)) {
        ALOGE("stop called in an invalid state: %d", mCurrentState);
 return INVALID_OPERATION;
 }

 status_t ret = mMediaRecorder->stop();
 if (OK != ret) {
        ALOGE("stop failed: %d", ret);
        mCurrentState = MEDIA_RECORDER_ERROR;
 return ret;
 }

    doCleanUp();
    mCurrentState = MEDIA_RECORDER_IDLE;
 return ret;
}
