status_t AudioFlinger::EffectModule::setOffloaded(bool offloaded, audio_io_handle_t io)
{
 Mutex::Autolock _l(mLock);
 if (mStatus != NO_ERROR) {
 return mStatus;
 }
 status_t status = NO_ERROR;
 if ((mDescriptor.flags & EFFECT_FLAG_OFFLOAD_SUPPORTED) != 0) {
 status_t cmdStatus;
 uint32_t size = sizeof(status_t);
 effect_offload_param_t cmd;

        cmd.isOffload = offloaded;
        cmd.ioHandle = io;
        status = (*mEffectInterface)->command(mEffectInterface,
                                              EFFECT_CMD_OFFLOAD,
 sizeof(effect_offload_param_t),
 &cmd,
 &size,
 &cmdStatus);
 if (status == NO_ERROR) {
            status = cmdStatus;
 }
        mOffloaded = (status == NO_ERROR) ? offloaded : false;
 } else {
 if (offloaded) {
            status = INVALID_OPERATION;
 }
        mOffloaded = false;
 }
    ALOGV("setOffloaded() offloaded %d io %d status %d", offloaded, io, status);
 return status;
}
