static size_t getMinTimestampIdx(OMX_S64 *pNTimeStamp, bool *pIsTimeStampValid) {
    OMX_S64 minTimeStamp = LLONG_MAX;
 int idx = -1;
 for (size_t i = 0; i < MAX_TIME_STAMPS; i++) {
 if (pIsTimeStampValid[i]) {
 if (pNTimeStamp[i] < minTimeStamp) {
                minTimeStamp = pNTimeStamp[i];
                idx = i;
 }
 }
 }
 return idx;
}
