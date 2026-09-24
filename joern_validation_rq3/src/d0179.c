void Splash::pipeRun(SplashPipe *pipe) {
  Guchar aSrc, aDest, alphaI, alphaIm1, alpha0, aResult;
  SplashColor cSrcNonIso, cDest, cBlend;
  SplashColorPtr cSrc;
  Guchar cResult0, cResult1, cResult2, cResult3;
  int t;
#if SPLASH_CMYK
  int cp, mask;
  Guchar cResult[SPOT_NCOMPS+4];
#endif



  if (pipe->pattern) {
    if (!pipe->pattern->getColor(pipe->x, pipe->y, pipe->cSrcVal)) {
		pipeIncX(pipe);
		return;
    }
  }

  if (pipe->noTransparency && !state->blendFunc) {


    switch (bitmap->mode) {
    case splashModeMono1:
      cResult0 = state->grayTransfer[pipe->cSrc[0]];
      if (state->screen->test(pipe->x, pipe->y, cResult0)) {
	*pipe->destColorPtr |= pipe->destColorMask;
      } else {
	*pipe->destColorPtr &= ~pipe->destColorMask;
      }
      if (!(pipe->destColorMask >>= 1)) {
	pipe->destColorMask = 0x80;
	++pipe->destColorPtr;
      }
      break;
    case splashModeMono8:
      *pipe->destColorPtr++ = state->grayTransfer[pipe->cSrc[0]];
      break;
    case splashModeRGB8:
      *pipe->destColorPtr++ = state->rgbTransferR[pipe->cSrc[0]];
      *pipe->destColorPtr++ = state->rgbTransferG[pipe->cSrc[1]];
      *pipe->destColorPtr++ = state->rgbTransferB[pipe->cSrc[2]];
      break;
    case splashModeXBGR8:
      *pipe->destColorPtr++ = state->rgbTransferB[pipe->cSrc[2]];
      *pipe->destColorPtr++ = state->rgbTransferG[pipe->cSrc[1]];
      *pipe->destColorPtr++ = state->rgbTransferR[pipe->cSrc[0]];
      *pipe->destColorPtr++ = 255;
      break;
    case splashModeBGR8:
      *pipe->destColorPtr++ = state->rgbTransferB[pipe->cSrc[2]];
      *pipe->destColorPtr++ = state->rgbTransferG[pipe->cSrc[1]];
      *pipe->destColorPtr++ = state->rgbTransferR[pipe->cSrc[0]];
      break;
#if SPLASH_CMYK
    case splashModeCMYK8:
      if (state->overprintMask & 1) {
	pipe->destColorPtr[0] = (state->overprintAdditive) ? 
              std::min<int>(pipe->destColorPtr[0] + state->cmykTransferC[pipe->cSrc[0]], 255) :
              state->cmykTransferC[pipe->cSrc[0]];
      }
      if (state->overprintMask & 2) {
	pipe->destColorPtr[1] = (state->overprintAdditive) ? 
              std::min<int>(pipe->destColorPtr[1] + state->cmykTransferM[pipe->cSrc[1]], 255) :
              state->cmykTransferM[pipe->cSrc[1]];
      }
      if (state->overprintMask & 4) {
	pipe->destColorPtr[2] = (state->overprintAdditive) ? 
              std::min<int>(pipe->destColorPtr[2] + state->cmykTransferY[pipe->cSrc[2]], 255) :
              state->cmykTransferY[pipe->cSrc[2]];
      }
      if (state->overprintMask & 8) {
	pipe->destColorPtr[3] = (state->overprintAdditive) ? 
              std::min<int>(pipe->destColorPtr[3] + state->cmykTransferK[pipe->cSrc[3]], 255) :
              state->cmykTransferK[pipe->cSrc[3]];
      }
      pipe->destColorPtr += 4;
      break;
    case splashModeDeviceN8:
      mask = 1;
      for (cp = 0; cp < SPOT_NCOMPS + 4; cp ++) {
        if (state->overprintMask & mask) {
          pipe->destColorPtr[cp] = state->deviceNTransfer[cp][pipe->cSrc[cp]];
        }
        mask <<= 1;
      }
      pipe->destColorPtr += (SPOT_NCOMPS+4);
      break;
#endif
    }
    if (pipe->destAlphaPtr) {
      *pipe->destAlphaPtr++ = 255;
    }

  } else {


    switch (bitmap->mode) {
    case splashModeMono1:
      cDest[0] = (*pipe->destColorPtr & pipe->destColorMask) ? 0xff : 0x00;
      break;
    case splashModeMono8:
      cDest[0] = *pipe->destColorPtr;
      break;
    case splashModeRGB8:
      cDest[0] = pipe->destColorPtr[0];
      cDest[1] = pipe->destColorPtr[1];
      cDest[2] = pipe->destColorPtr[2];
      break;
    case splashModeXBGR8:
      cDest[0] = pipe->destColorPtr[2];
      cDest[1] = pipe->destColorPtr[1];
      cDest[2] = pipe->destColorPtr[0];
      cDest[3] = 255;
      break;
    case splashModeBGR8:
      cDest[0] = pipe->destColorPtr[2];
      cDest[1] = pipe->destColorPtr[1];
      cDest[2] = pipe->destColorPtr[0];
      break;
#if SPLASH_CMYK
    case splashModeCMYK8:
      cDest[0] = pipe->destColorPtr[0];
      cDest[1] = pipe->destColorPtr[1];
      cDest[2] = pipe->destColorPtr[2];
      cDest[3] = pipe->destColorPtr[3];
      break;
    case splashModeDeviceN8:
      for (cp = 0; cp < SPOT_NCOMPS + 4; cp++)
        cDest[cp] = pipe->destColorPtr[cp];
      break;
#endif
    }
    if (pipe->destAlphaPtr) {
      aDest = *pipe->destAlphaPtr;
    } else {
      aDest = 0xff;
    }


    if (state->softMask) {
      if (pipe->usesShape) {
	aSrc = div255(div255(pipe->aInput * *pipe->softMaskPtr++) *
		      pipe->shape);
      } else {
	aSrc = div255(pipe->aInput * *pipe->softMaskPtr++);
      }
    } else if (pipe->usesShape) {
      aSrc = div255(pipe->aInput * pipe->shape);
    } else {
      aSrc = pipe->aInput;
    }


    if (pipe->nonIsolatedGroup) {
      if (pipe->shape == 0) {
	cSrc = pipe->cSrc;
      } else {
	t = (aDest * 255) / pipe->shape - aDest;
	switch (bitmap->mode) {
#if SPLASH_CMYK
	case splashModeDeviceN8:
	  for (cp = 4; cp < SPOT_NCOMPS + 4; cp++)
	    cSrcNonIso[cp] = clip255(pipe->cSrc[cp] +
				  ((pipe->cSrc[cp] - cDest[cp]) * t) / 255);
	case splashModeCMYK8:
	  cSrcNonIso[3] = clip255(pipe->cSrc[3] +
				  ((pipe->cSrc[3] - cDest[3]) * t) / 255);
#endif
	case splashModeRGB8:
  case splashModeXBGR8:
	case splashModeBGR8:
	  cSrcNonIso[2] = clip255(pipe->cSrc[2] +
				  ((pipe->cSrc[2] - cDest[2]) * t) / 255);
	  cSrcNonIso[1] = clip255(pipe->cSrc[1] +
				  ((pipe->cSrc[1] - cDest[1]) * t) / 255);
	case splashModeMono1:
	case splashModeMono8:
	  cSrcNonIso[0] = clip255(pipe->cSrc[0] +
				  ((pipe->cSrc[0] - cDest[0]) * t) / 255);
	  break;
	}
	cSrc = cSrcNonIso;
        if (pipe->knockout && pipe->shape >= pipe->knockoutOpacity) {
          aDest = 0;
        }
      }
    } else {
      cSrc = pipe->cSrc;
    }


    if (state->blendFunc) {
      (*state->blendFunc)(cSrc, cDest, cBlend, bitmap->mode);
    }


    if (pipe->noTransparency) {
      alphaI = alphaIm1 = aResult = 255;
    } else {
      aResult = aSrc + aDest - div255(aSrc * aDest);

      if (pipe->alpha0Ptr) {
	alpha0 = *pipe->alpha0Ptr++;
	alphaI = aResult + alpha0 - div255(aResult * alpha0);
	alphaIm1 = alpha0 + aDest - div255(alpha0 * aDest);
      } else {
	alphaI = aResult;
	alphaIm1 = aDest;
      }
    }


    cResult0 = cResult1 = cResult2 = cResult3 = 0; // make gcc happy

    switch (pipe->resultColorCtrl) {

    case splashPipeResultColorNoAlphaBlendMono:
      cResult0 = state->grayTransfer[div255((255 - aDest) * cSrc[0] +
					    aDest * cBlend[0])];
      break;
    case splashPipeResultColorNoAlphaBlendRGB:
      cResult0 = state->rgbTransferR[div255((255 - aDest) * cSrc[0] +
					    aDest * cBlend[0])];
      cResult1 = state->rgbTransferG[div255((255 - aDest) * cSrc[1] +
					    aDest * cBlend[1])];
      cResult2 = state->rgbTransferB[div255((255 - aDest) * cSrc[2] +
					    aDest * cBlend[2])];
      break;
#if SPLASH_CMYK
    case splashPipeResultColorNoAlphaBlendCMYK:
      cResult0 = state->cmykTransferC[div255((255 - aDest) * cSrc[0] +
					     aDest * cBlend[0])];
      cResult1 = state->cmykTransferM[div255((255 - aDest) * cSrc[1] +
					     aDest * cBlend[1])];
      cResult2 = state->cmykTransferY[div255((255 - aDest) * cSrc[2] +
					     aDest * cBlend[2])];
      cResult3 = state->cmykTransferK[div255((255 - aDest) * cSrc[3] +
					     aDest * cBlend[3])];
      break;
    case splashPipeResultColorNoAlphaBlendDeviceN:
      for (cp = 0; cp < SPOT_NCOMPS+4; cp++)
        cResult[cp] = state->deviceNTransfer[cp][div255((255 - aDest) * cSrc[cp] +
					     aDest * cBlend[cp])];
      break;
#endif

    case splashPipeResultColorAlphaNoBlendMono:
      if (alphaI == 0) {
	cResult0 = 0;
      } else {
	cResult0 = state->grayTransfer[((alphaI - aSrc) * cDest[0] +
					aSrc * cSrc[0]) / alphaI];
      }
      break;
    case splashPipeResultColorAlphaNoBlendRGB:
      if (alphaI == 0) {
	cResult0 = 0;
	cResult1 = 0;
	cResult2 = 0;
      } else {
	cResult0 = state->rgbTransferR[((alphaI - aSrc) * cDest[0] +
					aSrc * cSrc[0]) / alphaI];
	cResult1 = state->rgbTransferG[((alphaI - aSrc) * cDest[1] +
					aSrc * cSrc[1]) / alphaI];
	cResult2 = state->rgbTransferB[((alphaI - aSrc) * cDest[2] +
					aSrc * cSrc[2]) / alphaI];
      }
      break;
#if SPLASH_CMYK
    case splashPipeResultColorAlphaNoBlendCMYK:
      if (alphaI == 0) {
	cResult0 = 0;
	cResult1 = 0;
	cResult2 = 0;
	cResult3 = 0;
      } else {
	cResult0 = state->cmykTransferC[((alphaI - aSrc) * cDest[0] +
					 aSrc * cSrc[0]) / alphaI];
	cResult1 = state->cmykTransferM[((alphaI - aSrc) * cDest[1] +
					 aSrc * cSrc[1]) / alphaI];
	cResult2 = state->cmykTransferY[((alphaI - aSrc) * cDest[2] +
					 aSrc * cSrc[2]) / alphaI];
	cResult3 = state->cmykTransferK[((alphaI - aSrc) * cDest[3] +
					 aSrc * cSrc[3]) / alphaI];
      }
      break;
    case splashPipeResultColorAlphaNoBlendDeviceN:
      if (alphaI == 0) {
        for (cp = 0; cp < SPOT_NCOMPS+4; cp++)
          cResult[cp] = 0;
      } else {
        for (cp = 0; cp < SPOT_NCOMPS+4; cp++)
          cResult[cp] = state->deviceNTransfer[cp][((alphaI - aSrc) * cDest[cp] +
					 aSrc * cSrc[cp]) / alphaI];
      }
      break;
#endif

    case splashPipeResultColorAlphaBlendMono:
      if (alphaI == 0) {
	cResult0 = 0;
      } else {
	cResult0 = state->grayTransfer[((alphaI - aSrc) * cDest[0] +
					aSrc * ((255 - alphaIm1) * cSrc[0] +
						alphaIm1 * cBlend[0]) / 255) /
				       alphaI];
      }
      break;
    case splashPipeResultColorAlphaBlendRGB:
      if (alphaI == 0) {
	cResult0 = 0;
	cResult1 = 0;
	cResult2 = 0;
      } else {
	cResult0 = state->rgbTransferR[((alphaI - aSrc) * cDest[0] +
					aSrc * ((255 - alphaIm1) * cSrc[0] +
						alphaIm1 * cBlend[0]) / 255) /
				       alphaI];
	cResult1 = state->rgbTransferG[((alphaI - aSrc) * cDest[1] +
					aSrc * ((255 - alphaIm1) * cSrc[1] +
						alphaIm1 * cBlend[1]) / 255) /
				       alphaI];
	cResult2 = state->rgbTransferB[((alphaI - aSrc) * cDest[2] +
					aSrc * ((255 - alphaIm1) * cSrc[2] +
						alphaIm1 * cBlend[2]) / 255) /
				       alphaI];
      }
      break;
#if SPLASH_CMYK
    case splashPipeResultColorAlphaBlendCMYK:
      if (alphaI == 0) {
	cResult0 = 0;
	cResult1 = 0;
	cResult2 = 0;
	cResult3 = 0;
      } else {
	cResult0 = state->cmykTransferC[((alphaI - aSrc) * cDest[0] +
					 aSrc * ((255 - alphaIm1) * cSrc[0] +
						 alphaIm1 * cBlend[0]) / 255) /
					alphaI];
	cResult1 = state->cmykTransferM[((alphaI - aSrc) * cDest[1] +
					 aSrc * ((255 - alphaIm1) * cSrc[1] +
						 alphaIm1 * cBlend[1]) / 255) /
					alphaI];
	cResult2 = state->cmykTransferY[((alphaI - aSrc) * cDest[2] +
					 aSrc * ((255 - alphaIm1) * cSrc[2] +
						 alphaIm1 * cBlend[2]) / 255) /
					alphaI];
	cResult3 = state->cmykTransferK[((alphaI - aSrc) * cDest[3] +
					 aSrc * ((255 - alphaIm1) * cSrc[3] +
						 alphaIm1 * cBlend[3]) / 255) /
					alphaI];
      }
      break;
    case splashPipeResultColorAlphaBlendDeviceN:
      if (alphaI == 0) {
        for (cp = 0; cp < SPOT_NCOMPS+4; cp++)
          cResult[cp] = 0;
      } else {
        for (cp = 0; cp < SPOT_NCOMPS+4; cp++)
          cResult[cp] = state->deviceNTransfer[cp][((alphaI - aSrc) * cDest[cp] +
            aSrc * ((255 - alphaIm1) * cSrc[cp] +
						alphaIm1 * cBlend[cp]) / 255) /
					  alphaI];
      }
      break;
#endif
    }


    switch (bitmap->mode) {
    case splashModeMono1:
      if (state->screen->test(pipe->x, pipe->y, cResult0)) {
	*pipe->destColorPtr |= pipe->destColorMask;
      } else {
	*pipe->destColorPtr &= ~pipe->destColorMask;
      }
      if (!(pipe->destColorMask >>= 1)) {
	pipe->destColorMask = 0x80;
	++pipe->destColorPtr;
      }
      break;
    case splashModeMono8:
      *pipe->destColorPtr++ = cResult0;
      break;
    case splashModeRGB8:
      *pipe->destColorPtr++ = cResult0;
      *pipe->destColorPtr++ = cResult1;
      *pipe->destColorPtr++ = cResult2;
      break;
    case splashModeXBGR8:
      *pipe->destColorPtr++ = cResult2;
      *pipe->destColorPtr++ = cResult1;
      *pipe->destColorPtr++ = cResult0;
      *pipe->destColorPtr++ = 255;
      break;
    case splashModeBGR8:
      *pipe->destColorPtr++ = cResult2;
      *pipe->destColorPtr++ = cResult1;
      *pipe->destColorPtr++ = cResult0;
      break;
#if SPLASH_CMYK
    case splashModeCMYK8:
      if (state->overprintMask & 1) {
	pipe->destColorPtr[0] = (state->overprintAdditive) ? 
              std::min<int>(pipe->destColorPtr[0] + cResult0, 255) :
              cResult0;
      }
      if (state->overprintMask & 2) {
	pipe->destColorPtr[1] = (state->overprintAdditive) ? 
              std::min<int>(pipe->destColorPtr[1] + cResult1, 255) :
              cResult1;
      }
      if (state->overprintMask & 4) {
	pipe->destColorPtr[2] = (state->overprintAdditive) ? 
              std::min<int>(pipe->destColorPtr[2] + cResult2, 255) :
              cResult2;
      }
      if (state->overprintMask & 8) {
	pipe->destColorPtr[3] = (state->overprintAdditive) ? 
              std::min<int>(pipe->destColorPtr[3] + cResult3, 255) :
              cResult3;
      }
      pipe->destColorPtr += 4;
      break;
    case splashModeDeviceN8:
      mask = 1;
      for (cp = 0; cp < SPOT_NCOMPS+4; cp++) {
        if (state->overprintMask & mask) {
          pipe->destColorPtr[cp] = cResult[cp];
        }
        mask <<=1;
      }
      pipe->destColorPtr += (SPOT_NCOMPS+4);
      break;
#endif
    }
    if (pipe->destAlphaPtr) {
      *pipe->destAlphaPtr++ = aResult;
    }

  }

  ++pipe->x;
}
