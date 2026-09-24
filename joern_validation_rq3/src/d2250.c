static MagickBooleanType DecodeImage(Image *image,
  const MagickBooleanType compression,unsigned char *pixels)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__MINGW32__) || defined(__MINGW64__)
#define BI_RGB  0
#define BI_RLE8  1
#define BI_RLE4  2
#define BI_BITFIELDS  3
#endif

  int
    count;

  ssize_t
    y;

  register ssize_t
    i,
    x;

  register unsigned char
    *p,
    *q;

  unsigned char
    byte;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(pixels != (unsigned char *) NULL);
  (void) ResetMagickMemory(pixels,0,(size_t) image->columns*image->rows*
    sizeof(*pixels));
  byte=0;
  x=0;
  p=pixels;
  q=pixels+(size_t) image->columns*image->rows;
  for (y=0; y < (ssize_t) image->rows; )
  {
    if ((p < pixels) || (p >= q))
      break;
    count=ReadBlobByte(image);
    if (count == EOF)
      break;
    if (count != 0)
      {
        count=(int) MagickMin((size_t) count,(size_t) (q-p));
        /*
          Encoded mode.
        */
        byte=(unsigned char) ReadBlobByte(image);
        if (compression == BI_RLE8)
          {
            for (i=0; i < count; i++)
              *p++=(unsigned char) byte;
          }
        else
          {
            for (i=0; i < count; i++)
              *p++=(unsigned char)
                ((i & 0x01) != 0 ? (byte & 0x0f) : ((byte >> 4) & 0x0f));
          }
        x+=count;
      }
    else
      {
        /*
          Escape mode.
        */
        count=ReadBlobByte(image);
        if (count == 0x01)
          return(MagickTrue);
        switch (count)
        {
          case 0x00:
          {
            /*
              End of line.
            */
            x=0;
            y++;
            p=pixels+y*image->columns;
            break;
          }
          case 0x02:
          {
            /*
              Delta mode.
            */
            x+=ReadBlobByte(image);
            y+=ReadBlobByte(image);
            p=pixels+y*image->columns+x;
            break;
          }
          default:
          {
            /*
              Absolute mode.
            */
            count=(int) MagickMin((size_t) count,(size_t) (q-p));
            if (compression == BI_RLE8)
              for (i=0; i < count; i++)
                *p++=(unsigned char) ReadBlobByte(image);
            else
              for (i=0; i < count; i++)
              {
                if ((i & 0x01) == 0)
                  byte=(unsigned char) ReadBlobByte(image);
                *p++=(unsigned char)
                  ((i & 0x01) != 0 ? (byte & 0x0f) : ((byte >> 4) & 0x0f));
              }
            x+=count;
            /*
              Read pad byte.
            */
            if (compression == BI_RLE8)
              {
                if ((count & 0x01) != 0)
                  (void) ReadBlobByte(image);
              }
            else
              if (((count & 0x03) == 1) || ((count & 0x03) == 2))
                (void) ReadBlobByte(image);
            break;
          }
        }
      }
    if (SetImageProgress(image,LoadImageTag,y,image->rows) == MagickFalse)
      break;
  }
  (void) ReadBlobByte(image);  /* end of line */
  (void) ReadBlobByte(image);
  return(MagickTrue);
}
