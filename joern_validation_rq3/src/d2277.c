inverseFilteringLevelEmphasis(HANDLE_SBR_LPP_TRANS hLppTrans,/*!< Handle of lpp transposer  */
                              UCHAR nInvfBands, /*!< Number of bands for inverse filtering */
                              INVF_MODE *sbr_invf_mode, /*!< Current inverse filtering modes */
                              INVF_MODE *sbr_invf_mode_prev, /*!< Previous inverse filtering modes */
                              FIXP_DBL * bwVector            /*!< Resulting filtering levels */
 )
{
 for(int i = 0; i < nInvfBands; i++) {
    FIXP_DBL accu;
    FIXP_DBL bwTmp = mapInvfMode (sbr_invf_mode[i],
                                  sbr_invf_mode_prev[i],
                                  hLppTrans->pSettings->whFactors);

 if(bwTmp < hLppTrans->bwVectorOld[i]) {
      accu = fMultDiv2(FL2FXCONST_DBL(0.75f),bwTmp) +
             fMultDiv2(FL2FXCONST_DBL(0.25f),hLppTrans->bwVectorOld[i]);
 }
 else {
      accu = fMultDiv2(FL2FXCONST_DBL(0.90625f),bwTmp) +
             fMultDiv2(FL2FXCONST_DBL(0.09375f),hLppTrans->bwVectorOld[i]);
 }

 if (accu <  FL2FXCONST_DBL(0.015625f)>>1)
      bwVector[i] = FL2FXCONST_DBL(0.0f);
 else
      bwVector[i] = fixMin(accu<<1,FL2FXCONST_DBL(0.99609375f));
 }
}
