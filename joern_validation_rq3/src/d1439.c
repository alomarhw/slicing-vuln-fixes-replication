static MagickStatusType CorrectPSDOpacity(LayerInfo* layer_info,
  ExceptionInfo *exception)
{
  register PixelPacket
    *q;

  register ssize_t
    x;

  ssize_t
    y;

  if (layer_info->opacity == OpaqueOpacity)
    return(MagickTrue);

  layer_info->image->matte=MagickTrue;
  for (y=0; y < (ssize_t) layer_info->image->rows; y++)
  {
    q=GetAuthenticPixels(layer_info->image,0,y,layer_info->image->columns,1,
      exception);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (ssize_t) layer_info->image->columns; x++)
    {
      q->opacity=(Quantum) (QuantumRange-(Quantum) (QuantumScale*(
        (QuantumRange-q->opacity)*(QuantumRange-layer_info->opacity))));
      q++;
    }
    if (SyncAuthenticPixels(layer_info->image,exception) == MagickFalse)
      return(MagickFalse);
  }
  return(MagickTrue);
}
