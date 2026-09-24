MagickExport IndexPacket *GetAuthenticIndexQueue(const Image *image)
{
  CacheInfo
    *magick_restrict cache_info;

  const int
    id = GetOpenMPThreadId();

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image->cache != (Cache) NULL);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if (cache_info->methods.get_authentic_indexes_from_handler !=
       (GetAuthenticIndexesFromHandler) NULL)
    return(cache_info->methods.get_authentic_indexes_from_handler(image));
  assert(id < (int) cache_info->number_threads);
  return(cache_info->nexus_info[id]->indexes);
}
