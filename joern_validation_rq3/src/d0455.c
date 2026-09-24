status_t Camera2Client::updateRequests(Parameters &params) {
 status_t res;

    ALOGV("%s: Camera %d: state = %d", __FUNCTION__, getCameraId(), params.state);

    res = mStreamingProcessor->incrementStreamingIds();
 if (res != OK) {
        ALOGE("%s: Camera %d: Unable to increment request IDs: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
 return res;
 }

    res = mStreamingProcessor->updatePreviewRequest(params);
 if (res != OK) {
        ALOGE("%s: Camera %d: Unable to update preview request: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
 return res;
 }
    res = mStreamingProcessor->updateRecordingRequest(params);
 if (res != OK) {
        ALOGE("%s: Camera %d: Unable to update recording request: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
 return res;
 }

 if (params.state == Parameters::PREVIEW) {
        res = startPreviewL(params, true);
 if (res != OK) {
            ALOGE("%s: Camera %d: Error streaming new preview request: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
 return res;
 }
 } else if (params.state == Parameters::RECORD ||
            params.state == Parameters::VIDEO_SNAPSHOT) {
        res = startRecordingL(params, true);
 if (res != OK) {
            ALOGE("%s: Camera %d: Error streaming new record request: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
 return res;
 }
 }
 return res;
}
