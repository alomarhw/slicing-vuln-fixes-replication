  hash_free( hashtable*  ht,
             FT_Memory   memory )
  {
    if ( ht != 0 )
    {
      int        i, sz = ht->size;
      hashnode*  bp = ht->table;


      for ( i = 0; i < sz; i++, bp++ )
        FT_FREE( *bp );

      FT_FREE( ht->table );
    }
  }
