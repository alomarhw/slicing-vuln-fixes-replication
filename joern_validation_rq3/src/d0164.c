is_v2_ckpt( const char *name )
{
	int		cluster;
	int		proc;

	cluster = grab_val( name, "job" );
	proc = grab_val( name, ".ckpt." );

	if( proc < 0 ) {
		return cluster_exists( cluster );
	} else {
		return proc_exists( cluster, proc );
	}
}
