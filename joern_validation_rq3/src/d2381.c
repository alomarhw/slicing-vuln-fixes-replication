status_t ACodec::allocateOutputBuffersFromNativeWindow() {
    OMX_U32 bufferCount, bufferSize, minUndequeuedBuffers;
 status_t err = configureOutputBuffersFromNativeWindow(
 &bufferCount, &bufferSize, &minUndequeuedBuffers);
 if (err != 0)
 return err;
    mNumUndequeuedBuffers = minUndequeuedBuffers;

 if (!storingMetadataInDecodedBuffers()) {
 static_cast<Surface*>(mNativeWindow.get())
 ->getIGraphicBufferProducer()->allowAllocation(true);
 }

    ALOGV("[%s] Allocating %u buffers from a native window of size %u on "
 "output port",
         mComponentName.c_str(), bufferCount, bufferSize);

 for (OMX_U32 i = 0; i < bufferCount; i++) {
 ANativeWindowBuffer *buf;
 int fenceFd;
        err = mNativeWindow->dequeueBuffer(mNativeWindow.get(), &buf, &fenceFd);
 if (err != 0) {
            ALOGE("dequeueBuffer failed: %s (%d)", strerror(-err), -err);
 break;
 }

        sp<GraphicBuffer> graphicBuffer(new GraphicBuffer(buf, false));
 BufferInfo info;
        info.mStatus = BufferInfo::OWNED_BY_US;
        info.mFenceFd = fenceFd;
        info.mIsReadFence = false;
        info.mRenderInfo = NULL;
        info.mData = new ABuffer(NULL /* data */, bufferSize /* capacity */);
        info.mGraphicBuffer = graphicBuffer;
        mBuffers[kPortIndexOutput].push(info);

        IOMX::buffer_id bufferId;
        err = mOMX->useGraphicBuffer(mNode, kPortIndexOutput, graphicBuffer,
 &bufferId);
 if (err != 0) {
            ALOGE("registering GraphicBuffer %u with OMX IL component failed: "
 "%d", i, err);
 break;
 }

        mBuffers[kPortIndexOutput].editItemAt(i).mBufferID = bufferId;

        ALOGV("[%s] Registered graphic buffer with ID %u (pointer = %p)",
             mComponentName.c_str(),
             bufferId, graphicBuffer.get());
 }

    OMX_U32 cancelStart;
    OMX_U32 cancelEnd;

 if (err != 0) {
        cancelStart = 0;
        cancelEnd = mBuffers[kPortIndexOutput].size();
 } else {
        cancelStart = bufferCount - minUndequeuedBuffers;
        cancelEnd = bufferCount;
 }

 for (OMX_U32 i = cancelStart; i < cancelEnd; i++) {
 BufferInfo *info = &mBuffers[kPortIndexOutput].editItemAt(i);
 if (info->mStatus == BufferInfo::OWNED_BY_US) {
 status_t error = cancelBufferToNativeWindow(info);
 if (err == 0) {
                err = error;
 }
 }
 }

 if (!storingMetadataInDecodedBuffers()) {
 static_cast<Surface*>(mNativeWindow.get())
 ->getIGraphicBufferProducer()->allowAllocation(false);
 }

 return err;
}
