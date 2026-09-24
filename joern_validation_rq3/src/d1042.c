GfxColorSpace *GfxDeviceNColorSpace::copy() {
  GfxDeviceNColorSpace *cs;
  int i;

  cs = new GfxDeviceNColorSpace(nComps, alt->copy(), func->copy());
  for (i = 0; i < nComps; ++i) {
    cs->names[i] = names[i]->copy();
  }
  cs->nonMarking = nonMarking;
  return cs;
}
