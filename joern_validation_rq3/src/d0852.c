MagickExport PixelPacket *QueueAuthenticPixels(Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,
  ExceptionInfo *exception)
{
  CacheInfo
    *restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->methods.queue_authentic_pixels_handler !=
       (QueueAuthenticPixelsHandler) NULL)
    return(cache_info->methods.queue_authentic_pixels_handler(image,x,y,columns,
      rows,exception));
  assert(id < (int) cache_info->number_threads);
  return(QueueAuthenticPixelCacheNexus(image,x,y,columns,rows,MagickFalse,
    cache_info->nexus_info[id],exception));
}
