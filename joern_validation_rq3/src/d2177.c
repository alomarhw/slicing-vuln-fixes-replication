void OMXCodec::initOutputFormat(const sp<MetaData> &inputFormat) {
    mOutputFormat = new MetaData;
    mOutputFormat->setCString(kKeyDecoderComponent, mComponentName);
 if (mIsEncoder) {
 int32_t timeScale;
 if (inputFormat->findInt32(kKeyTimeScale, &timeScale)) {
            mOutputFormat->setInt32(kKeyTimeScale, timeScale);
 }
 }

    OMX_PARAM_PORTDEFINITIONTYPE def;
 InitOMXParams(&def);
    def.nPortIndex = kPortIndexOutput;

 status_t err = mOMX->getParameter(
            mNode, OMX_IndexParamPortDefinition, &def, sizeof(def));
    CHECK_EQ(err, (status_t)OK);

 switch (def.eDomain) {
 case OMX_PortDomainImage:
 {
            OMX_IMAGE_PORTDEFINITIONTYPE *imageDef = &def.format.image;
            CHECK_EQ((int)imageDef->eCompressionFormat,
 (int)OMX_IMAGE_CodingUnused);

            mOutputFormat->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_RAW);
            mOutputFormat->setInt32(kKeyColorFormat, imageDef->eColorFormat);
            mOutputFormat->setInt32(kKeyWidth, imageDef->nFrameWidth);
            mOutputFormat->setInt32(kKeyHeight, imageDef->nFrameHeight);
 break;
 }

 case OMX_PortDomainAudio:
 {
            OMX_AUDIO_PORTDEFINITIONTYPE *audio_def = &def.format.audio;

 if (audio_def->eEncoding == OMX_AUDIO_CodingPCM) {
                OMX_AUDIO_PARAM_PCMMODETYPE params;
 InitOMXParams(&params);
                params.nPortIndex = kPortIndexOutput;

                err = mOMX->getParameter(
                        mNode, OMX_IndexParamAudioPcm, &params, sizeof(params));
                CHECK_EQ(err, (status_t)OK);

                CHECK_EQ((int)params.eNumData, (int)OMX_NumericalDataSigned);
                CHECK_EQ(params.nBitPerSample, 16u);
                CHECK_EQ((int)params.ePCMMode, (int)OMX_AUDIO_PCMModeLinear);

 int32_t numChannels, sampleRate;
                inputFormat->findInt32(kKeyChannelCount, &numChannels);
                inputFormat->findInt32(kKeySampleRate, &sampleRate);

 if ((OMX_U32)numChannels != params.nChannels) {
                    ALOGV("Codec outputs a different number of channels than "
 "the input stream contains (contains %d channels, "
 "codec outputs %u channels).",
                         numChannels, params.nChannels);
 }

 if (sampleRate != (int32_t)params.nSamplingRate) {
                    ALOGV("Codec outputs at different sampling rate than "
 "what the input stream contains (contains data at "
 "%d Hz, codec outputs %u Hz)",
                         sampleRate, params.nSamplingRate);
 }

                mOutputFormat->setCString(
                        kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_RAW);

                mOutputFormat->setInt32(
                        kKeyChannelCount,
 (mQuirks & kDecoderLiesAboutNumberOfChannels)
 ? numChannels : params.nChannels);

                mOutputFormat->setInt32(kKeySampleRate, params.nSamplingRate);
 } else if (audio_def->eEncoding == OMX_AUDIO_CodingAMR) {
                OMX_AUDIO_PARAM_AMRTYPE amr;
 InitOMXParams(&amr);
                amr.nPortIndex = kPortIndexOutput;

                err = mOMX->getParameter(
                        mNode, OMX_IndexParamAudioAmr, &amr, sizeof(amr));
                CHECK_EQ(err, (status_t)OK);

                CHECK_EQ(amr.nChannels, 1u);
                mOutputFormat->setInt32(kKeyChannelCount, 1);

 if (amr.eAMRBandMode >= OMX_AUDIO_AMRBandModeNB0
 && amr.eAMRBandMode <= OMX_AUDIO_AMRBandModeNB7) {
                    mOutputFormat->setCString(
                            kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_AMR_NB);
                    mOutputFormat->setInt32(kKeySampleRate, 8000);
 } else if (amr.eAMRBandMode >= OMX_AUDIO_AMRBandModeWB0
 && amr.eAMRBandMode <= OMX_AUDIO_AMRBandModeWB8) {
                    mOutputFormat->setCString(
                            kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_AMR_WB);
                    mOutputFormat->setInt32(kKeySampleRate, 16000);
 } else {
                    CHECK(!"Unknown AMR band mode.");
 }
 } else if (audio_def->eEncoding == OMX_AUDIO_CodingAAC) {
                mOutputFormat->setCString(
                        kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_AAC);
 int32_t numChannels, sampleRate, bitRate;
                inputFormat->findInt32(kKeyChannelCount, &numChannels);
                inputFormat->findInt32(kKeySampleRate, &sampleRate);
                inputFormat->findInt32(kKeyBitRate, &bitRate);
                mOutputFormat->setInt32(kKeyChannelCount, numChannels);
                mOutputFormat->setInt32(kKeySampleRate, sampleRate);
                mOutputFormat->setInt32(kKeyBitRate, bitRate);
 } else if (audio_def->eEncoding ==
 (OMX_AUDIO_CODINGTYPE)OMX_AUDIO_CodingAndroidAC3) {
                mOutputFormat->setCString(
                        kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_AC3);
 int32_t numChannels, sampleRate, bitRate;
                inputFormat->findInt32(kKeyChannelCount, &numChannels);
                inputFormat->findInt32(kKeySampleRate, &sampleRate);
                inputFormat->findInt32(kKeyBitRate, &bitRate);
                mOutputFormat->setInt32(kKeyChannelCount, numChannels);
                mOutputFormat->setInt32(kKeySampleRate, sampleRate);
                mOutputFormat->setInt32(kKeyBitRate, bitRate);
 } else {
                CHECK(!"Should not be here. Unknown audio encoding.");
 }
 break;
 }

 case OMX_PortDomainVideo:
 {
            OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &def.format.video;

 if (video_def->eCompressionFormat == OMX_VIDEO_CodingUnused) {
                mOutputFormat->setCString(
                        kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_RAW);
 } else if (video_def->eCompressionFormat == OMX_VIDEO_CodingMPEG4) {
                mOutputFormat->setCString(
                        kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_MPEG4);
 } else if (video_def->eCompressionFormat == OMX_VIDEO_CodingH263) {
                mOutputFormat->setCString(
                        kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_H263);
 } else if (video_def->eCompressionFormat == OMX_VIDEO_CodingAVC) {
                mOutputFormat->setCString(
                        kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_AVC);
 } else {
                CHECK(!"Unknown compression format.");
 }

            mOutputFormat->setInt32(kKeyWidth, video_def->nFrameWidth);
            mOutputFormat->setInt32(kKeyHeight, video_def->nFrameHeight);
            mOutputFormat->setInt32(kKeyColorFormat, video_def->eColorFormat);

 if (!mIsEncoder) {
                OMX_CONFIG_RECTTYPE rect;
 InitOMXParams(&rect);
                rect.nPortIndex = kPortIndexOutput;
 status_t err =
                        mOMX->getConfig(
                            mNode, OMX_IndexConfigCommonOutputCrop,
 &rect, sizeof(rect));

                CODEC_LOGI("video dimensions are %u x %u",
                        video_def->nFrameWidth, video_def->nFrameHeight);

 if (err == OK) {
                    CHECK_GE(rect.nLeft, 0);
                    CHECK_GE(rect.nTop, 0);
                    CHECK_GE(rect.nWidth, 0u);
                    CHECK_GE(rect.nHeight, 0u);
                    CHECK_LE(rect.nLeft + rect.nWidth - 1, video_def->nFrameWidth);
                    CHECK_LE(rect.nTop + rect.nHeight - 1, video_def->nFrameHeight);

                    mOutputFormat->setRect(
                            kKeyCropRect,
                            rect.nLeft,
                            rect.nTop,
                            rect.nLeft + rect.nWidth - 1,
                            rect.nTop + rect.nHeight - 1);

                    CODEC_LOGI("Crop rect is %u x %u @ (%d, %d)",
                            rect.nWidth, rect.nHeight, rect.nLeft, rect.nTop);
 } else {
                    mOutputFormat->setRect(
                            kKeyCropRect,
 0, 0,
                            video_def->nFrameWidth - 1,
                            video_def->nFrameHeight - 1);
 }

 if (mNativeWindow != NULL) {
                     initNativeWindowCrop();
 }
 }
 break;
 }

 default:
 {
            CHECK(!"should not be here, neither audio nor video.");
 break;
 }
 }


 int32_t rotationDegrees;
 if (mSource->getFormat()->findInt32(kKeyRotation, &rotationDegrees)) {
        mOutputFormat->setInt32(kKeyRotation, rotationDegrees);
 }
}
