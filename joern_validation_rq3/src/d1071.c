wkbConvPolygonToShape(wkbObj *w, shapeObj *shape)
{
  int type;
  int i, nrings;
  lineObj line;

  /*endian = */wkbReadChar(w);
  type = wkbTypeMap(w,wkbReadInt(w));

  if( type != WKB_POLYGON ) return MS_FAILURE;

  /* How many rings? */
  nrings = wkbReadInt(w);

  /* Add each ring to the shape */
  for( i = 0; i < nrings; i++ ) {
    wkbReadLine(w,&line);
    msAddLineDirectly(shape, &line);
  }

  return MS_SUCCESS;
}
