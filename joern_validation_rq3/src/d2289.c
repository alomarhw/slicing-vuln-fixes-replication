void CairoOutputDev::updateLineDash(GfxState *state) {
  double *dashPattern;
  int dashLength;
  double dashStart;

  state->getLineDash(&dashPattern, &dashLength, &dashStart);
  cairo_set_dash (cairo, dashPattern, dashLength, dashStart);
  if (cairo_shape)
    cairo_set_dash (cairo_shape, dashPattern, dashLength, dashStart);
}
