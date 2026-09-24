int jpc_tsfb_synthesize2(jpc_tsfb_t *tsfb, int *a, int xstart, int ystart,
  int width, int height, int stride, int numlvls)
{
	if (numlvls > 0) {
		if (jpc_tsfb_synthesize2(tsfb, a, JPC_CEILDIVPOW2(xstart, 1),
		  JPC_CEILDIVPOW2(ystart, 1), JPC_CEILDIVPOW2(xstart + width,
		  1) - JPC_CEILDIVPOW2(xstart, 1), JPC_CEILDIVPOW2(ystart +
		  height, 1) - JPC_CEILDIVPOW2(ystart, 1), stride, numlvls -
		  1)) {
			return -1;
		}
	}
	if (width > 0 && height > 0) {
		if ((*tsfb->qmfb->synthesize)(a, xstart, ystart, width, height, stride)) {
			return -1;
		}
	}
	return 0;
}
