static inline void CopyBlob(Image *source,Image *destination)
{
  ssize_t
    i;

  unsigned char
    *buffer;

  ssize_t
    count,
    length;

  buffer=(unsigned char *) AcquireQuantumMemory(MagickMaxBufferExtent,
    sizeof(*buffer));
  if (buffer != (unsigned char *) NULL)
    {
      i=0;
      while ((length=ReadBlob(source,MagickMaxBufferExtent,buffer)) != 0)
      {
        count=0;
        for (i=0; i < (ssize_t) length; i+=count)
        {
          count=WriteBlob(destination,(size_t) (length-i),buffer+i);
          if (count <= 0)
            break;
        }
        if (i < (ssize_t) length)
          break;
      }
      buffer=(unsigned char *) RelinquishMagickMemory(buffer);
    }
}
