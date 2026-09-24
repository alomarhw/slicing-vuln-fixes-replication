static inline unsigned int ablkcipher_done_fast(struct ablkcipher_walk *walk,
						unsigned int n)
{
	scatterwalk_advance(&walk->in, n);
	scatterwalk_advance(&walk->out, n);

	return n;
}
