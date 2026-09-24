static int mount_smb2_usage(FILE *stream)
{
	fprintf(stream, "\nUsage:  %s <remotetarget> <dir> -o <options>\n",
		thisprogram);
	fprintf(stream, "\nMount the remote target, specified as a UNC name,");
	fprintf(stream, " to a local directory.\n\nOptions:\n");
	fprintf(stream, "\tuser=<arg>\n\tpass=<arg>\n\tdom=<arg>\n");
	fprintf(stream, "\nLess commonly used options:");
	fprintf(stream,
		"\n\tcredentials=<filename>,guest,perm,noperm,rw,ro,");
	fprintf(stream,
		"\n\tsep=<char>,iocharset=<codepage>,exec,noexec");
	fprintf(stream,
		"\n\tnolock,directio,sec=<authentication mechanism>,sign");
	fprintf(stream,
		"\n\tuid=<uid>,gid=<gid>,dir_mode=<mode>,file_mode=<mode>");
	fprintf(stream, "\n\nRarely used options:");
	fprintf(stream,
		"\n\tport=<tcpport>,rsize=<size>,wsize=<size>,unc=<unc_name>,ip=<ip_address>,");
	fprintf(stream,
		"\n\tdev,nodev,hard,soft,intr,");
	fprintf(stream,
		"\n\tnointr,ignorecase,noacl,prefixpath=<path>,nobrl");
	fprintf(stream,
		"\n\nOptions are described in more detail in the manual page");
	fprintf(stream, "\n\tman 8 mount.smb2\n");
	fprintf(stream, "\nTo display the version number of the mount helper:");
	fprintf(stream, "\n\tmount.smb2 -V\n");

	if (stream == stderr)
		return EX_USAGE;
	return 0;
}
