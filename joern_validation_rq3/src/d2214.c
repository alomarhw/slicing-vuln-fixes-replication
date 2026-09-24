ModuleExport size_t RegisterMPCImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("MPC","CACHE",
    "Magick Persistent Cache image format");
  entry->flags|=CoderStealthFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("MPC","MPC","Magick Persistent Cache image format");
  entry->decoder=(DecodeImageHandler *) ReadMPCImage;
  entry->encoder=(EncodeImageHandler *) WriteMPCImage;
  entry->magick=(IsImageFormatHandler *) IsMPC;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}
