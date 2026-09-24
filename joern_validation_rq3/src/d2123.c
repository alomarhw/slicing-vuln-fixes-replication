status_t SoftMPEG2::reInitDecoder() {
 status_t ret;

    deInitDecoder();

    ret = initDecoder();
 if (OK != ret) {
        ALOGE("Create failure");
        deInitDecoder();
 return NO_MEMORY;
 }
 return OK;
}
