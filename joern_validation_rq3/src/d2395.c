static int MagickRealTypeCompare(const void *error_p,const void *error_q)
{
  MagickRealType
    *p,
    *q;

  p=(MagickRealType *) error_p;
  q=(MagickRealType *) error_q;
  if (*p > *q)
    return(1);
  if (fabs((double) (*q-*p)) <= MagickEpsilon)
    return(0);
  return(-1);
}
