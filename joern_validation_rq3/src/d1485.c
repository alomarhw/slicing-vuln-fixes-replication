static PolygonInfo **AcquirePolygonThreadSet(
  const PrimitiveInfo *primitive_info)
{
  PathInfo
    *magick_restrict path_info;

  PolygonInfo
    **polygon_info;

  register ssize_t
    i;

  size_t
    number_threads;

  number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  polygon_info=(PolygonInfo **) AcquireQuantumMemory(number_threads,
    sizeof(*polygon_info));
  if (polygon_info == (PolygonInfo **) NULL)
    return((PolygonInfo **) NULL);
  (void) ResetMagickMemory(polygon_info,0,number_threads*sizeof(*polygon_info));
  path_info=ConvertPrimitiveToPath(primitive_info);
  if (path_info == (PathInfo *) NULL)
    return(DestroyPolygonThreadSet(polygon_info));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    polygon_info[i]=ConvertPathToPolygon(path_info);
    if (polygon_info[i] == (PolygonInfo *) NULL)
      return(DestroyPolygonThreadSet(polygon_info));
  }
  path_info=(PathInfo *) RelinquishMagickMemory(path_info);
  return(polygon_info);
}
