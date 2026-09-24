MagickExport MagickBooleanType RemapImages(const QuantizeInfo *quantize_info,
  Image *images,const Image *remap_image)
{
  CubeInfo
    *cube_info;

  Image
    *image;

  MagickBooleanType
    status;

  assert(images != (Image *) NULL);
  assert(images->signature == MagickSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  image=images;
  if (remap_image == (Image *) NULL)
    {
      /*
        Create a global colormap for an image sequence.
      */
      status=QuantizeImages(quantize_info,images);
      return(status);
    }
  /*
    Classify image colors from the reference image.
  */
  cube_info=GetCubeInfo(quantize_info,MaxTreeDepth,
    quantize_info->number_colors);
  if (cube_info == (CubeInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  status=ClassifyImageColors(cube_info,remap_image,&image->exception);
  if (status != MagickFalse)
    {
      /*
        Classify image colors from the reference image.
      */
      cube_info->quantize_info->number_colors=cube_info->colors;
      image=images;
      for ( ; image != (Image *) NULL; image=GetNextImageInList(image))
      {
        status=AssignImageColors(image,cube_info);
        if (status == MagickFalse)
          break;
      }
    }
  DestroyCubeInfo(cube_info);
  return(status);
}
