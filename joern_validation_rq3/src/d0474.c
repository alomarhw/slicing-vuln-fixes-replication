static MagickBooleanType WBMPReadInteger(Image *image,size_t *value)
{
  int
    byte;

  *value=0;
  do
  {
    byte=ReadBlobByte(image);
    if (byte == EOF)
      return(MagickFalse);
    *value<<=7;
    *value|=(unsigned int) (byte & 0x7f);
  } while (byte & 0x80);
  return(MagickTrue);
}
