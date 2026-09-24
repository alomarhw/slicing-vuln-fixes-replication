  Direct_Move_Orig( TT_ExecContext  exc,
                    TT_GlyphZone    zone,
                    FT_UShort       point,
                    FT_F26Dot6      distance )
  {
    FT_F26Dot6  v;


    v = exc->GS.freeVector.x;

    if ( v != 0 )
      zone->org[point].x = ADD_LONG( zone->org[point].x,
                                     FT_MulDiv( distance,
                                                v,
                                                exc->F_dot_P ) );

    v = exc->GS.freeVector.y;

    if ( v != 0 )
      zone->org[point].y = ADD_LONG( zone->org[point].y,
                                     FT_MulDiv( distance,
                                                v,
                                                exc->F_dot_P ) );
  }
