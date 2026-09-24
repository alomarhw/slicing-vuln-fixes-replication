status_t OMXNodeInstance::createInputSurface(
        OMX_U32 portIndex, android_dataspace dataSpace,
        sp<IGraphicBufferProducer> *bufferProducer, MetadataBufferType *type) {
 if (bufferProducer == NULL) {
        ALOGE("b/25884056");
 return BAD_VALUE;
 }

 Mutex::Autolock autolock(mLock);
 status_t err = createGraphicBufferSource(portIndex, NULL /* bufferConsumer */, type);

 if (err != OK) {
 return err;
 }

    mGraphicBufferSource->setDefaultDataSpace(dataSpace);

 *bufferProducer = mGraphicBufferSource->getIGraphicBufferProducer();
 return OK;
}
