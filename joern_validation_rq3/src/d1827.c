static void _S_AL_SanitiseVector( vec3_t v, int line )
{
	if( Q_isnan( v[ 0 ] ) || Q_isnan( v[ 1 ] ) || Q_isnan( v[ 2 ] ) )
	{
		Com_DPrintf( S_COLOR_YELLOW "WARNING: vector with one or more NaN components "
				"being passed to OpenAL at %s:%d -- zeroing\n", __FILE__, line );
		VectorClear( v );
	}
}
