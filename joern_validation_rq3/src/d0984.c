u32 h264bsdDecode(storage_t *pStorage, u8 *byteStrm, u32 len, u32 picId,
    u32 *readBytes)
{

/* Variables */

    u32 tmp, ppsId, spsId;
    i32 picOrderCnt;
 nalUnit_t nalUnit;
 seqParamSet_t seqParamSet;
 picParamSet_t picParamSet;
 strmData_t strm;
    u32 accessUnitBoundaryFlag = HANTRO_FALSE;
    u32 picReady = HANTRO_FALSE;

/* Code */

    ASSERT(pStorage);
    ASSERT(byteStrm);
    ASSERT(len);
    ASSERT(readBytes);

 /* if previous buffer was not finished and same pointer given -> skip NAL
     * unit extraction */
 if (pStorage->prevBufNotFinished && byteStrm == pStorage->prevBufPointer)
 {
        strm = pStorage->strm[0];
        strm.pStrmCurrPos = strm.pStrmBuffStart;
        strm.strmBuffReadBits = strm.bitPosInWord = 0;
 *readBytes = pStorage->prevBytesConsumed;
 }
 else
 {
        tmp = h264bsdExtractNalUnit(byteStrm, len, &strm, readBytes);
 if (tmp != HANTRO_OK)
 {
            EPRINT("BYTE_STREAM");
 return(H264BSD_ERROR);
 }
 /* store stream */
        pStorage->strm[0] = strm;
        pStorage->prevBytesConsumed = *readBytes;
        pStorage->prevBufPointer = byteStrm;
 }
    pStorage->prevBufNotFinished = HANTRO_FALSE;

    tmp = h264bsdDecodeNalUnit(&strm, &nalUnit);
 if (tmp != HANTRO_OK)
 {
        EPRINT("NAL_UNIT");
 return(H264BSD_ERROR);
 }

 /* Discard unspecified, reserved, SPS extension and auxiliary picture slices */
 if(nalUnit.nalUnitType == 0 || nalUnit.nalUnitType >= 13)
 {
        DEBUG(("DISCARDED NAL (UNSPECIFIED, REGISTERED, SPS ext or AUX slice)\n"));
 return(H264BSD_RDY);
 }

    tmp = h264bsdCheckAccessUnitBoundary(
 &strm,
 &nalUnit,
      pStorage,
 &accessUnitBoundaryFlag);
 if (tmp != HANTRO_OK)
 {
        EPRINT("ACCESS UNIT BOUNDARY CHECK");
 if (tmp == PARAM_SET_ERROR)
 return(H264BSD_PARAM_SET_ERROR);
 else
 return(H264BSD_ERROR);
 }

 if ( accessUnitBoundaryFlag )
 {
        DEBUG(("Access unit boundary\n"));
 /* conceal if picture started and param sets activated */
 if (pStorage->picStarted && pStorage->activeSps != NULL)
 {
            DEBUG(("CONCEALING..."));

 /* return error if second phase of
             * initialization is not completed */
 if (pStorage->pendingActivation)
 {
                EPRINT("Pending activation not completed");
 return (H264BSD_ERROR);
 }

 if (!pStorage->validSliceInAccessUnit)
 {
                pStorage->currImage->data =
                    h264bsdAllocateDpbImage(pStorage->dpb);
                h264bsdInitRefPicList(pStorage->dpb);
                tmp = h264bsdConceal(pStorage, pStorage->currImage, P_SLICE);
 }
 else
                tmp = h264bsdConceal(pStorage, pStorage->currImage,
                    pStorage->sliceHeader->sliceType);

            picReady = HANTRO_TRUE;

 /* current NAL unit should be decoded on next activation -> set
             * readBytes to 0 */
 *readBytes = 0;
            pStorage->prevBufNotFinished = HANTRO_TRUE;
            DEBUG(("...DONE\n"));
 }
 else
 {
            pStorage->validSliceInAccessUnit = HANTRO_FALSE;
 }
        pStorage->skipRedundantSlices = HANTRO_FALSE;
 }

 if (!picReady)
 {
 switch (nalUnit.nalUnitType)
 {
 case NAL_SEQ_PARAM_SET:
                DEBUG(("SEQ PARAM SET\n"));
                tmp = h264bsdDecodeSeqParamSet(&strm, &seqParamSet);
 if (tmp != HANTRO_OK)
 {
                    EPRINT("SEQ_PARAM_SET");
                    FREE(seqParamSet.offsetForRefFrame);
                    FREE(seqParamSet.vuiParameters);
 return(H264BSD_ERROR);
 }
                tmp = h264bsdStoreSeqParamSet(pStorage, &seqParamSet);
 break;

 case NAL_PIC_PARAM_SET:
                DEBUG(("PIC PARAM SET\n"));
                tmp = h264bsdDecodePicParamSet(&strm, &picParamSet);
 if (tmp != HANTRO_OK)
 {
                    EPRINT("PIC_PARAM_SET");
                    FREE(picParamSet.runLength);
                    FREE(picParamSet.topLeft);
                    FREE(picParamSet.bottomRight);
                    FREE(picParamSet.sliceGroupId);
 return(H264BSD_ERROR);
 }
                tmp = h264bsdStorePicParamSet(pStorage, &picParamSet);
 break;

 case NAL_CODED_SLICE_IDR:
                DEBUG(("IDR "));
 /* fall through */
 case NAL_CODED_SLICE:
                DEBUG(("SLICE HEADER\n"));

 /* picture successfully finished and still decoding same old
                 * access unit -> no need to decode redundant slices */
 if (pStorage->skipRedundantSlices)
 return(H264BSD_RDY);

                pStorage->picStarted = HANTRO_TRUE;

 if (h264bsdIsStartOfPicture(pStorage))
 {
                    pStorage->numConcealedMbs = 0;
                    pStorage->currentPicId    = picId;

                    tmp = h264bsdCheckPpsId(&strm, &ppsId);
                    ASSERT(tmp == HANTRO_OK);
 /* store old activeSpsId and return headers ready
                     * indication if activeSps changes */
                    spsId = pStorage->activeSpsId;
                    tmp = h264bsdActivateParamSets(pStorage, ppsId,
                            IS_IDR_NAL_UNIT(&nalUnit) ?
                            HANTRO_TRUE : HANTRO_FALSE);
 if (tmp != HANTRO_OK)
 {
                        EPRINT("Param set activation");
                        pStorage->activePpsId = MAX_NUM_PIC_PARAM_SETS;
                        pStorage->activePps = NULL;
                        pStorage->activeSpsId = MAX_NUM_SEQ_PARAM_SETS;
                        pStorage->activeSps = NULL;
                        pStorage->pendingActivation = HANTRO_FALSE;

 if(tmp == MEMORY_ALLOCATION_ERROR)
 {
 return H264BSD_MEMALLOC_ERROR;
 }
 else
 return(H264BSD_PARAM_SET_ERROR);
 }

 if (spsId != pStorage->activeSpsId)
 {
 seqParamSet_t *oldSPS = NULL;
 seqParamSet_t *newSPS = pStorage->activeSps;
                        u32 noOutputOfPriorPicsFlag = 1;

 if(pStorage->oldSpsId < MAX_NUM_SEQ_PARAM_SETS)
 {
                            oldSPS = pStorage->sps[pStorage->oldSpsId];
 }

 *readBytes = 0;
                        pStorage->prevBufNotFinished = HANTRO_TRUE;


 if(nalUnit.nalUnitType == NAL_CODED_SLICE_IDR)
 {
                            tmp =
                            h264bsdCheckPriorPicsFlag(&noOutputOfPriorPicsFlag,
 &strm, newSPS,
                                                          pStorage->activePps,
                                                          nalUnit.nalUnitType);
 }
 else
 {
                            tmp = HANTRO_NOK;
 }

 if((tmp != HANTRO_OK) ||
 (noOutputOfPriorPicsFlag != 0) ||
 (pStorage->dpb->noReordering) ||
 (oldSPS == NULL) ||
 (oldSPS->picWidthInMbs != newSPS->picWidthInMbs) ||
 (oldSPS->picHeightInMbs != newSPS->picHeightInMbs) ||
 (oldSPS->maxDpbSize != newSPS->maxDpbSize))
 {
                            pStorage->dpb->flushed = 0;
 }
 else
 {
                            h264bsdFlushDpb(pStorage->dpb);
 }

                        pStorage->oldSpsId = pStorage->activeSpsId;

 return(H264BSD_HDRS_RDY);
 }
 }

 /* return error if second phase of
                 * initialization is not completed */
 if (pStorage->pendingActivation)
 {
                    EPRINT("Pending activation not completed");
 return (H264BSD_ERROR);
 }
                tmp = h264bsdDecodeSliceHeader(&strm, pStorage->sliceHeader + 1,
                    pStorage->activeSps, pStorage->activePps, &nalUnit);
 if (tmp != HANTRO_OK)
 {
                    EPRINT("SLICE_HEADER");
 return(H264BSD_ERROR);
 }
 if (h264bsdIsStartOfPicture(pStorage))
 {
 if (!IS_IDR_NAL_UNIT(&nalUnit))
 {
                        tmp = h264bsdCheckGapsInFrameNum(pStorage->dpb,
                            pStorage->sliceHeader[1].frameNum,
                            nalUnit.nalRefIdc != 0 ?
                            HANTRO_TRUE : HANTRO_FALSE,
                            pStorage->activeSps->
                            gapsInFrameNumValueAllowedFlag);
 if (tmp != HANTRO_OK)
 {
                            EPRINT("Gaps in frame num");
 return(H264BSD_ERROR);
 }
 }
                    pStorage->currImage->data =
                        h264bsdAllocateDpbImage(pStorage->dpb);
 }

 /* store slice header to storage if successfully decoded */
                pStorage->sliceHeader[0] = pStorage->sliceHeader[1];
                pStorage->validSliceInAccessUnit = HANTRO_TRUE;
                pStorage->prevNalUnit[0] = nalUnit;

                h264bsdComputeSliceGroupMap(pStorage,
                    pStorage->sliceHeader->sliceGroupChangeCycle);

                h264bsdInitRefPicList(pStorage->dpb);
                tmp = h264bsdReorderRefPicList(pStorage->dpb,
 &pStorage->sliceHeader->refPicListReordering,
                    pStorage->sliceHeader->frameNum,
                    pStorage->sliceHeader->numRefIdxL0Active);
 if (tmp != HANTRO_OK)
 {
                    EPRINT("Reordering");
 return(H264BSD_ERROR);
 }

                DEBUG(("SLICE DATA, FIRST %d\n",
                        pStorage->sliceHeader->firstMbInSlice));
                tmp = h264bsdDecodeSliceData(&strm, pStorage,
                    pStorage->currImage, pStorage->sliceHeader);
 if (tmp != HANTRO_OK)
 {
                    EPRINT("SLICE_DATA");
                    h264bsdMarkSliceCorrupted(pStorage,
                        pStorage->sliceHeader->firstMbInSlice);
 return(H264BSD_ERROR);
 }

 if (h264bsdIsEndOfPicture(pStorage))
 {
                    picReady = HANTRO_TRUE;
                    pStorage->skipRedundantSlices = HANTRO_TRUE;
 }
 break;

 case NAL_SEI:
                DEBUG(("SEI MESSAGE, NOT DECODED"));
 break;

 default:
                DEBUG(("NOT IMPLEMENTED YET %d\n",nalUnit.nalUnitType));
 }
 }

 if (picReady)
 {
        h264bsdFilterPicture(pStorage->currImage, pStorage->mb);

        h264bsdResetStorage(pStorage);

        picOrderCnt = h264bsdDecodePicOrderCnt(pStorage->poc,
            pStorage->activeSps, pStorage->sliceHeader, pStorage->prevNalUnit);

 if (pStorage->validSliceInAccessUnit)
 {
 if (pStorage->prevNalUnit->nalRefIdc)
 {
                tmp = h264bsdMarkDecRefPic(pStorage->dpb,
 &pStorage->sliceHeader->decRefPicMarking,
                    pStorage->currImage, pStorage->sliceHeader->frameNum,
                    picOrderCnt,
                    IS_IDR_NAL_UNIT(pStorage->prevNalUnit) ?
                    HANTRO_TRUE : HANTRO_FALSE,
                    pStorage->currentPicId, pStorage->numConcealedMbs);
 }
 /* non-reference picture, just store for possible display
             * reordering */
 else
 {
                tmp = h264bsdMarkDecRefPic(pStorage->dpb, NULL,
                    pStorage->currImage, pStorage->sliceHeader->frameNum,
                    picOrderCnt,
                    IS_IDR_NAL_UNIT(pStorage->prevNalUnit) ?
                    HANTRO_TRUE : HANTRO_FALSE,
                    pStorage->currentPicId, pStorage->numConcealedMbs);
 }
 }

        pStorage->picStarted = HANTRO_FALSE;
        pStorage->validSliceInAccessUnit = HANTRO_FALSE;

 return(H264BSD_PIC_RDY);
 }
 else
 return(H264BSD_RDY);

}
