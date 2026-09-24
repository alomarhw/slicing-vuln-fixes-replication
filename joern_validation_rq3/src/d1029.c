R_API void r_bin_iobind(RBin *bin, RIO *io) {
	r_io_bind (io, &bin->iob);
}
