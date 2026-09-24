  tt_cmap4_char_map_binary( TT_CMap     cmap,
                            FT_UInt32*  pcharcode,
                            FT_Bool     next )
  {
    FT_UInt   num_segs2, start, end, offset;
    FT_Int    delta;
    FT_UInt   max, min, mid, num_segs;
    FT_UInt   charcode = *pcharcode;
    FT_UInt   gindex   = 0;
    FT_Byte*  p;


    p = cmap->data + 6;
    num_segs2 = FT_PAD_FLOOR( TT_PEEK_USHORT( p ), 2 );

    if ( !num_segs2 )
      return 0;

    num_segs = num_segs2 >> 1;

    /* make compiler happy */
    mid = num_segs;
    end = 0xFFFFU;

    if ( next )
      charcode++;

    min = 0;
    max = num_segs;

    /* binary search */
    while ( min < max )
    {
      mid    = ( min + max ) >> 1;
      p      = cmap->data + 14 + mid * 2;
      end    = TT_PEEK_USHORT( p );
      p     += 2 + num_segs2;
      start  = TT_PEEK_USHORT( p );

      if ( charcode < start )
        max = mid;
      else if ( charcode > end )
        min = mid + 1;
      else
      {
        p     += num_segs2;
        delta  = TT_PEEK_SHORT( p );
        p     += num_segs2;
        offset = TT_PEEK_USHORT( p );

        /* some fonts have an incorrect last segment; */
        /* we have to catch it                        */
        if ( mid >= num_segs - 1                &&
             start == 0xFFFFU && end == 0xFFFFU )
        {
          TT_Face   face  = (TT_Face)cmap->cmap.charmap.face;
          FT_Byte*  limit = face->cmap_table + face->cmap_size;


          if ( offset && p + offset + 2 > limit )
          {
            delta  = 1;
            offset = 0;
          }
        }

        /* search the first segment containing `charcode' */
        if ( cmap->flags & TT_CMAP_FLAG_OVERLAPPING )
        {
          FT_UInt  i;


          /* call the current segment `max' */
          max = mid;

          if ( offset == 0xFFFFU )
            mid = max + 1;

          /* search in segments before the current segment */
          for ( i = max ; i > 0; i-- )
          {
            FT_UInt   prev_end;
            FT_Byte*  old_p;


            old_p    = p;
            p        = cmap->data + 14 + ( i - 1 ) * 2;
            prev_end = TT_PEEK_USHORT( p );

            if ( charcode > prev_end )
            {
              p = old_p;
              break;
            }

            end    = prev_end;
            p     += 2 + num_segs2;
            start  = TT_PEEK_USHORT( p );
            p     += num_segs2;
            delta  = TT_PEEK_SHORT( p );
            p     += num_segs2;
            offset = TT_PEEK_USHORT( p );

            if ( offset != 0xFFFFU )
              mid = i - 1;
          }

          /* no luck */
          if ( mid == max + 1 )
          {
            if ( i != max )
            {
              p      = cmap->data + 14 + max * 2;
              end    = TT_PEEK_USHORT( p );
              p     += 2 + num_segs2;
              start  = TT_PEEK_USHORT( p );
              p     += num_segs2;
              delta  = TT_PEEK_SHORT( p );
              p     += num_segs2;
              offset = TT_PEEK_USHORT( p );
            }

            mid = max;

            /* search in segments after the current segment */
            for ( i = max + 1; i < num_segs; i++ )
            {
              FT_UInt  next_end, next_start;


              p          = cmap->data + 14 + i * 2;
              next_end   = TT_PEEK_USHORT( p );
              p         += 2 + num_segs2;
              next_start = TT_PEEK_USHORT( p );

              if ( charcode < next_start )
                break;

              end    = next_end;
              start  = next_start;
              p     += num_segs2;
              delta  = TT_PEEK_SHORT( p );
              p     += num_segs2;
              offset = TT_PEEK_USHORT( p );

              if ( offset != 0xFFFFU )
                mid = i;
            }
            i--;

            /* still no luck */
            if ( mid == max )
            {
              mid = i;

              break;
            }
          }

          /* end, start, delta, and offset are for the i'th segment */
          if ( mid != i )
          {
            p      = cmap->data + 14 + mid * 2;
            end    = TT_PEEK_USHORT( p );
            p     += 2 + num_segs2;
            start  = TT_PEEK_USHORT( p );
            p     += num_segs2;
            delta  = TT_PEEK_SHORT( p );
            p     += num_segs2;
            offset = TT_PEEK_USHORT( p );
          }
        }
        else
        {
          if ( offset == 0xFFFFU )
            break;
        }

        if ( offset )
        {
          p += offset + ( charcode - start ) * 2;
          gindex = TT_PEEK_USHORT( p );
          if ( gindex != 0 )
            gindex = (FT_UInt)( gindex + delta ) & 0xFFFFU;
        }
        else
          gindex = (FT_UInt)( charcode + delta ) & 0xFFFFU;

        break;
      }
    }

    if ( next )
    {
      TT_CMap4  cmap4 = (TT_CMap4)cmap;


      /* if `charcode' is not in any segment, then `mid' is */
      /* the segment nearest to `charcode'                  */
      /*                                                    */

      if ( charcode > end )
      {
        mid++;
        if ( mid == num_segs )
          return 0;
      }

      if ( tt_cmap4_set_range( cmap4, mid ) )
      {
        if ( gindex )
          *pcharcode = charcode;
      }
      else
      {
        cmap4->cur_charcode = charcode;

        if ( gindex )
          cmap4->cur_gindex = gindex;
        else
        {
          cmap4->cur_charcode = charcode;
          tt_cmap4_next( cmap4 );
          gindex = cmap4->cur_gindex;
        }

        if ( gindex )
          *pcharcode = cmap4->cur_charcode;
      }
    }

    return gindex;
  }
