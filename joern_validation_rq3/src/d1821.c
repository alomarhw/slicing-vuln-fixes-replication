  static Int  Current_Ppem( EXEC_OP )
  {
    return MulDiv_Round( CUR.metrics.ppem, CURRENT_Ratio(), 0x10000 );
  }
