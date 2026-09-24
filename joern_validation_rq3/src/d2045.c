void BrowserViewRenderer::ForceFakeCompositeSW() {
  DCHECK(compositor_);
  SkBitmap bitmap;
  bitmap.allocN32Pixels(1, 1);
  bitmap.eraseColor(0);
  SkCanvas canvas(bitmap);
  CompositeSW(&canvas);
}
