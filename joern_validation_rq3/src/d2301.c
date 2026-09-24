check_daemon_sock_dir()
{
	const char	*f;
	Directory dir(DaemonSockDir, PRIV_ROOT);

	time_t stale_age = SharedPortEndpoint::TouchSocketInterval()*100;

	while( (f = dir.Next()) ) {
		MyString full_path;
		full_path.sprintf("%s%c%s",DaemonSockDir,DIR_DELIM_CHAR,f);

		if( touched_recently( full_path.Value(), stale_age ) ) {
			good_file( DaemonSockDir, f );
		}
		else {
			bad_file( DaemonSockDir, f, dir );
		}
	}
}
