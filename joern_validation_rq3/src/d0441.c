ImageBitmap::ImageBitmap(const void* pixelData,
                         uint32_t width,
                         uint32_t height,
                         bool isImageBitmapPremultiplied,
                         bool isImageBitmapOriginClean) {
  SkImageInfo info = SkImageInfo::MakeN32(
      width, height,
      isImageBitmapPremultiplied ? kPremul_SkAlphaType : kUnpremul_SkAlphaType);
  SkPixmap pixmap(info, pixelData, info.bytesPerPixel() * width);
  m_image = StaticBitmapImage::create(SkImage::MakeRasterCopy(pixmap));
  if (!m_image)
    return;
  m_image->setPremultiplied(isImageBitmapPremultiplied);
  m_image->setOriginClean(isImageBitmapOriginClean);
}
