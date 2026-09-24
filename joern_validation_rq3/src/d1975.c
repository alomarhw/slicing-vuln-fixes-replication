static HB_Error  Load_PosRuleSet( HB_PosRuleSet*  prs,
				  HB_Stream        stream )
{
  HB_Error  error;

  HB_UShort     n, m, count;
  HB_UInt      cur_offset, new_offset, base_offset;

  HB_PosRule*  pr;


  base_offset = FILE_Pos();

  if ( ACCESS_Frame( 2L ) )
    return error;

  count = prs->PosRuleCount = GET_UShort();

  FORGET_Frame();

  prs->PosRule = NULL;

  if ( ALLOC_ARRAY( prs->PosRule, count, HB_PosRule ) )
    return error;

  pr = prs->PosRule;

  for ( n = 0; n < count; n++ )
  {
    if ( ACCESS_Frame( 2L ) )
      goto Fail;

    new_offset = GET_UShort() + base_offset;

    FORGET_Frame();

    cur_offset = FILE_Pos();
    if ( FILE_Seek( new_offset ) ||
	 ( error = Load_PosRule( &pr[n], stream ) ) != HB_Err_Ok )
      goto Fail;
    (void)FILE_Seek( cur_offset );
  }

  return HB_Err_Ok;

Fail:
  for ( m = 0; m < n; m++ )
    Free_PosRule( &pr[m] );

  FREE( pr );
  return error;
}
