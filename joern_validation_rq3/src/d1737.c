  _iup_worker_interpolate( IUP_Worker  worker,
                           FT_UInt     p1,
                           FT_UInt     p2,
                           FT_UInt     ref1,
                           FT_UInt     ref2 )
  {
    FT_UInt     i;
    FT_F26Dot6  orus1, orus2, org1, org2, delta1, delta2;


    if ( p1 > p2 )
      return;

    if ( BOUNDS( ref1, worker->max_points ) ||
         BOUNDS( ref2, worker->max_points ) )
      return;

    orus1 = worker->orus[ref1].x;
    orus2 = worker->orus[ref2].x;

    if ( orus1 > orus2 )
    {
      FT_F26Dot6  tmp_o;
      FT_UInt     tmp_r;


      tmp_o = orus1;
      orus1 = orus2;
      orus2 = tmp_o;

      tmp_r = ref1;
      ref1  = ref2;
      ref2  = tmp_r;
    }

    org1   = worker->orgs[ref1].x;
    org2   = worker->orgs[ref2].x;
    delta1 = worker->curs[ref1].x - org1;
    delta2 = worker->curs[ref2].x - org2;

    if ( orus1 == orus2 )
    {
      /* simple shift of untouched points */
      for ( i = p1; i <= p2; i++ )
      {
        FT_F26Dot6  x = worker->orgs[i].x;


        if ( x <= org1 )
          x += delta1;
        else
          x += delta2;

        worker->curs[i].x = x;
      }
    }
    else
    {
      FT_Fixed  scale       = 0;
      FT_Bool   scale_valid = 0;


      /* interpolation */
      for ( i = p1; i <= p2; i++ )
      {
        FT_F26Dot6  x = worker->orgs[i].x;


        if ( x <= org1 )
          x += delta1;

        else if ( x >= org2 )
          x += delta2;

        else
        {
          if ( !scale_valid )
          {
            scale_valid = 1;
            scale       = TT_MULDIV( org2 + delta2 - ( org1 + delta1 ),
                                     0x10000L, orus2 - orus1 );
          }

          x = ( org1 + delta1 ) +
              TT_MULFIX( worker->orus[i].x - orus1, scale );
        }
        worker->curs[i].x = x;
      }
    }
  }
