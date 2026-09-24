inline void Splash::pipeSetXY(SplashPipe *pipe, int x, int y) {
  pipe->x = x;
  pipe->y = y;
  if (state->softMask) {
    pipe->softMaskPtr =
        &state->softMask->data[y * state->softMask->rowSize + x];
  }
  switch (bitmap->mode) {
  case splashModeMono1:
    pipe->destColorPtr = &bitmap->data[y * bitmap->rowSize + (x >> 3)];
    pipe->destColorMask = 0x80 >> (x & 7);
    break;
  case splashModeMono8:
    pipe->destColorPtr = &bitmap->data[y * bitmap->rowSize + x];
    break;
  case splashModeRGB8:
  case splashModeBGR8:
    pipe->destColorPtr = &bitmap->data[y * bitmap->rowSize + 3 * x];
    break;
  case splashModeXBGR8:
    pipe->destColorPtr = &bitmap->data[y * bitmap->rowSize + 4 * x];
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    pipe->destColorPtr = &bitmap->data[y * bitmap->rowSize + 4 * x];
    break;
  case splashModeDeviceN8:
    pipe->destColorPtr = &bitmap->data[y * bitmap->rowSize + (SPOT_NCOMPS + 4) * x];
    break;
#endif
  }
  if (bitmap->alpha) {
    pipe->destAlphaPtr = &bitmap->alpha[y * bitmap->width + x];
  } else {
    pipe->destAlphaPtr = NULL;
  }
  if (state->inNonIsolatedGroup && alpha0Bitmap->alpha) {
    pipe->alpha0Ptr =
        &alpha0Bitmap->alpha[(alpha0Y + y) * alpha0Bitmap->width +
			     (alpha0X + x)];
  } else {
    pipe->alpha0Ptr = NULL;
  }
}
