  tt_cmap0_get_info( TT_CMap       cmap,
                     TT_CMapInfo  *cmap_info )
  {
    FT_Byte*  p = cmap->data + 4;


    cmap_info->format   = 0;
    cmap_info->language = (FT_ULong)TT_PEEK_USHORT( p );

    return FT_Err_Ok;
  }
