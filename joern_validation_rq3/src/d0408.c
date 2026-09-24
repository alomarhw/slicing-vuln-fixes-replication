status_t AudioFlinger::EffectModule::setVolume(uint32_t *left, uint32_t *right, bool controller)
{
 Mutex::Autolock _l(mLock);
 if (mStatus != NO_ERROR) {
 return mStatus;
 }
 status_t status = NO_ERROR;
 if (isProcessEnabled() &&
 ((mDescriptor.flags & EFFECT_FLAG_VOLUME_MASK) == EFFECT_FLAG_VOLUME_CTRL ||
 (mDescriptor.flags & EFFECT_FLAG_VOLUME_MASK) == EFFECT_FLAG_VOLUME_IND)) {
 status_t cmdStatus;
 uint32_t volume[2];
 uint32_t *pVolume = NULL;
 uint32_t size = sizeof(volume);
        volume[0] = *left;
        volume[1] = *right;
 if (controller) {
            pVolume = volume;
 }
        status = (*mEffectInterface)->command(mEffectInterface,
                                              EFFECT_CMD_SET_VOLUME,
                                              size,
                                              volume,
 &size,
                                              pVolume);
 if (controller && status == NO_ERROR && size == sizeof(volume)) {
 *left = volume[0];
 *right = volume[1];
 }
 }
 return status;
}
