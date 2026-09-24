diskfile_recv(struct iperf_stream *sp)
{
    int r;

    r = sp->rcv2(sp);
    if (r > 0) {
	(void) write(sp->diskfile_fd, sp->buffer, r);
	(void) fsync(sp->diskfile_fd);
    }
    return r;
}
