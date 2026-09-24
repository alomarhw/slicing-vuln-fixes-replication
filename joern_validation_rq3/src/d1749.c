void Con_Clear_f( void ) {
	int i;

	for ( i = 0 ; i < CON_TEXTSIZE ; i++ ) {
		con.text[i] = ( ColorIndex( COLNSOLE_COLOR ) << 8 ) | ' ';
	}

	Con_Bottom();       // go to end
}
