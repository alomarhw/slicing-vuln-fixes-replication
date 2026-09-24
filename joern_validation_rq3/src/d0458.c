uint32_t SampleTable::CompositionDeltaLookup::getCompositionTimeOffset(
 uint32_t sampleIndex) {
 Mutex::Autolock autolock(mLock);

 if (mDeltaEntries == NULL) {
 return 0;
 }

 if (sampleIndex < mCurrentEntrySampleIndex) {
        mCurrentDeltaEntry = 0;
        mCurrentEntrySampleIndex = 0;
 }

 while (mCurrentDeltaEntry < mNumDeltaEntries) {
 uint32_t sampleCount = mDeltaEntries[2 * mCurrentDeltaEntry];
 if (sampleIndex < mCurrentEntrySampleIndex + sampleCount) {
 return mDeltaEntries[2 * mCurrentDeltaEntry + 1];
 }

        mCurrentEntrySampleIndex += sampleCount;
 ++mCurrentDeltaEntry;
 }

 return 0;
}
