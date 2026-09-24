static void c_write_untrusted(Ssh ssh, const char *buf, int len)
{
    if (flags & FLAG_STDERR)
	c_write_stderr(0, buf, len);
    else
	from_backend_untrusted(ssh->frontend, buf, len);
}
