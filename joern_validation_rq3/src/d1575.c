int socket_shutdown(int fd, int how)
{
	return shutdown(fd, how);
}
