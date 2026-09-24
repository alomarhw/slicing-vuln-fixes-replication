static MagickBooleanType FloydSteinbergDither(Image *image,CubeInfo *cube_info)
{
#define DitherImageTag  "Dither/Image"

  CacheView
    *image_view;

  DoublePixelPacket
    **pixels;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  ssize_t
    y;

  /*
    Distribute quantization error using Floyd-Steinberg.
  */
  pixels=AcquirePixelThreadSet(image->columns);
  if (pixels == (DoublePixelPacket **) NULL)
    return(MagickFalse);
  exception=(&image->exception);
  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    CubeInfo
      cube;

    DoublePixelPacket
      *current,
      *previous;

    register IndexPacket
      *magick_restrict indexes;

    register PixelPacket
      *magick_restrict q;

    register ssize_t
      x;

    size_t
      index;

    ssize_t
      v;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    cube=(*cube_info);
    current=pixels[id]+(y & 0x01)*image->columns;
    previous=pixels[id]+((y+1) & 0x01)*image->columns;
    v=(ssize_t) ((y & 0x01) ? -1 : 1);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      DoublePixelPacket
        color,
        pixel;

      register ssize_t
        i;

      ssize_t
        u;

      u=(y & 0x01) ? (ssize_t) image->columns-1-x : x;
      AssociateAlphaPixel(&cube,q+u,&pixel);
      if (x > 0)
        {
          pixel.red+=7*current[u-v].red/16;
          pixel.green+=7*current[u-v].green/16;
          pixel.blue+=7*current[u-v].blue/16;
          if (cube.associate_alpha != MagickFalse)
            pixel.opacity+=7*current[u-v].opacity/16;
        }
      if (y > 0)
        {
          if (x < (ssize_t) (image->columns-1))
            {
              pixel.red+=previous[u+v].red/16;
              pixel.green+=previous[u+v].green/16;
              pixel.blue+=previous[u+v].blue/16;
              if (cube.associate_alpha != MagickFalse)
                pixel.opacity+=previous[u+v].opacity/16;
            }
          pixel.red+=5*previous[u].red/16;
          pixel.green+=5*previous[u].green/16;
          pixel.blue+=5*previous[u].blue/16;
          if (cube.associate_alpha != MagickFalse)
            pixel.opacity+=5*previous[u].opacity/16;
          if (x > 0)
            {
              pixel.red+=3*previous[u-v].red/16;
              pixel.green+=3*previous[u-v].green/16;
              pixel.blue+=3*previous[u-v].blue/16;
              if (cube.associate_alpha != MagickFalse)
                pixel.opacity+=3*previous[u-v].opacity/16;
            }
        }
      pixel.red=(MagickRealType) ClampPixel(pixel.red);
      pixel.green=(MagickRealType) ClampPixel(pixel.green);
      pixel.blue=(MagickRealType) ClampPixel(pixel.blue);
      if (cube.associate_alpha != MagickFalse)
        pixel.opacity=(MagickRealType) ClampPixel(pixel.opacity);
      i=CacheOffset(&cube,&pixel);
      if (cube.cache[i] < 0)
        {
          register NodeInfo
            *node_info;

          register size_t
            id;

          /*
            Identify the deepest node containing the pixel's color.
          */
          node_info=cube.root;
          for (index=MaxTreeDepth-1; (ssize_t) index > 0; index--)
          {
            id=ColorToNodeId(&cube,&pixel,index);
            if (node_info->child[id] == (NodeInfo *) NULL)
              break;
            node_info=node_info->child[id];
          }
          /*
            Find closest color among siblings and their children.
          */
          cube.target=pixel;
          cube.distance=(MagickRealType) (4.0*(QuantumRange+1.0)*(QuantumRange+
            1.0)+1.0);
          ClosestColor(image,&cube,node_info->parent);
          cube.cache[i]=(ssize_t) cube.color_number;
        }
      /*
        Assign pixel to closest colormap entry.
      */
      index=(size_t) cube.cache[i];
      if (image->storage_class == PseudoClass)
        SetPixelIndex(indexes+u,index);
      if (cube.quantize_info->measure_error == MagickFalse)
        {
          SetPixelRgb(q+u,image->colormap+index);
          if (cube.associate_alpha != MagickFalse)
            SetPixelOpacity(q+u,image->colormap[index].opacity);
        }
      if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
        status=MagickFalse;
      /*
        Store the error.
      */
      AssociateAlphaPixel(&cube,image->colormap+index,&color);
      current[u].red=pixel.red-color.red;
      current[u].green=pixel.green-color.green;
      current[u].blue=pixel.blue-color.blue;
      if (cube.associate_alpha != MagickFalse)
        current[u].opacity=pixel.opacity-color.opacity;
      if (image->progress_monitor != (MagickProgressMonitor) NULL)
        {
          MagickBooleanType
            proceed;

          proceed=SetImageProgress(image,DitherImageTag,(MagickOffsetType) y,
            image->rows);
          if (proceed == MagickFalse)
            status=MagickFalse;
        }
    }
  }
  image_view=DestroyCacheView(image_view);
  pixels=DestroyPixelThreadSet(pixels);
  return(MagickTrue);
}
