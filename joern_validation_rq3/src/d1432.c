short *XRRRates (Display *dpy, int screen, int sizeID, int *nrates)
{
  XRRScreenConfiguration *config;
  XExtDisplayInfo *info = XRRFindDisplay(dpy);
  short *rates;

  LockDisplay(dpy);
  if ((config = _XRRValidateCache(dpy, info, screen))) {
    rates = XRRConfigRates (config, sizeID, nrates);
    UnlockDisplay(dpy);
    return rates;
    }
  else {
    UnlockDisplay(dpy);
    *nrates = 0;
    return NULL;
  }
}
