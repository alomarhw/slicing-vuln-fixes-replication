ssize_t vfs_pwrite_data(struct smb_request *req,
			files_struct *fsp,
			const char *buffer,
			size_t N,
			off_t offset)
{
	size_t total=0;
	ssize_t ret;

	if (req && req->unread_bytes) {
		int sockfd = req->xconn->transport.sock;
		SMB_ASSERT(req->unread_bytes == N);
		/* VFS_RECVFILE must drain the socket
		 * before returning. */
		req->unread_bytes = 0;
		/*
		 * Leave the socket non-blocking and
		 * use SMB_VFS_RECVFILE. If it returns
		 * EAGAIN || EWOULDBLOCK temporarily set
		 * the socket blocking and retry
		 * the RECVFILE.
		 */
		while (total < N) {
			ret = SMB_VFS_RECVFILE(sockfd,
						fsp,
						offset + total,
						N - total);
			if (ret == 0 || (ret == -1 &&
					 (errno == EAGAIN ||
					  errno == EWOULDBLOCK))) {
				int old_flags;
				/* Ensure the socket is blocking. */
				old_flags = fcntl(sockfd, F_GETFL, 0);
				if (set_blocking(sockfd, true) == -1) {
					return (ssize_t)-1;
				}
				ret = SMB_VFS_RECVFILE(sockfd,
							fsp,
							offset + total,
							N - total);
				if (fcntl(sockfd, F_SETFL, old_flags) == -1) {
					return (ssize_t)-1;
				}
				if (ret == -1) {
					return (ssize_t)-1;
				}
				total += ret;
				return (ssize_t)total;
			}
			/* Any other error case. */
			if (ret == -1) {
				return ret;
			}
			total += ret;
		}
		return (ssize_t)total;
	}

	while (total < N) {
		ret = SMB_VFS_PWRITE(fsp, buffer + total, N - total,
				     offset + total);

		if (ret == -1)
			return -1;
		if (ret == 0)
			return total;

		total += ret;
	}
	return (ssize_t)total;
}
