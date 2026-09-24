static inline void RelinquishPixelCachePixels(CacheInfo *cache_info)
{
  switch (cache_info->type)
  {
    case MemoryCache:
    {
      if (cache_info->mapped == MagickFalse)
        cache_info->pixels=(PixelPacket *) RelinquishAlignedMemory(
          cache_info->pixels);
      else
        {
          (void) UnmapBlob(cache_info->pixels,(size_t) cache_info->length);
          cache_info->pixels=(PixelPacket *) NULL;
        }
      RelinquishMagickResource(MemoryResource,cache_info->length);
      break;
    }
    case MapCache:
    {
      (void) UnmapBlob(cache_info->pixels,(size_t) cache_info->length);
      cache_info->pixels=(PixelPacket *) NULL;
      if (cache_info->mode != ReadMode)
        (void) RelinquishUniqueFileResource(cache_info->cache_filename);
      *cache_info->cache_filename='\0';
      RelinquishMagickResource(MapResource,cache_info->length);
    }
    case DiskCache:
    {
      if (cache_info->file != -1)
        (void) ClosePixelCacheOnDisk(cache_info);
      if (cache_info->mode != ReadMode)
        (void) RelinquishUniqueFileResource(cache_info->cache_filename);
      *cache_info->cache_filename='\0';
      RelinquishMagickResource(DiskResource,cache_info->length);
      break;
    }
    case DistributedCache:
    {
      *cache_info->cache_filename='\0';
      (void) RelinquishDistributePixelCache((DistributeCacheInfo *)
        cache_info->server_info);
      break;
    }
    default:
      break;
  }
  cache_info->type=UndefinedCache;
  cache_info->mapped=MagickFalse;
  cache_info->indexes=(IndexPacket *) NULL;
}
