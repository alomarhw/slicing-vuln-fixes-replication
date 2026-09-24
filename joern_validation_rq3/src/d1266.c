void VirtualizerSetStrength(EffectContext *pContext, uint32_t strength){
 LVM_ControlParams_t ActiveParams; /* Current control Parameters */
    LVM_ReturnStatus_en     LvmStatus=LVM_SUCCESS; /* Function call status */

    pContext->pBundledContext->VirtStrengthSaved = (int)strength;

 /* Get the current settings */
 LvmStatus = LVM_GetControlParameters(pContext->pBundledContext->hInstance,&ActiveParams);

    LVM_ERROR_CHECK(LvmStatus, "LVM_GetControlParameters", "VirtualizerSetStrength")

 /* Virtualizer parameters */
 ActiveParams.CS_EffectLevel             = (int)((strength*32767)/1000);

    ALOGV("\tVirtualizerSetStrength() (0-1000)   -> %d\n", strength );
    ALOGV("\tVirtualizerSetStrength() (0- 100)   -> %d\n", ActiveParams.CS_EffectLevel );

 /* Activate the initial settings */
 LvmStatus = LVM_SetControlParameters(pContext->pBundledContext->hInstance, &ActiveParams);
    LVM_ERROR_CHECK(LvmStatus, "LVM_SetControlParameters", "VirtualizerSetStrength")
 LvmEffect_limitLevel(pContext);
} /* end setStrength */
