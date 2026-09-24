fbStore_b5g6r5 (FbBits *bits, const CARD32 *values, int x, int width, miIndexedPtr indexed)
{
    int i;
    CARD16  *pixel = ((CARD16 *) bits) + x;
    for (i = 0; i < width; ++i) {
        Split(READ(values + i));
        WRITE(pixel++, ((b << 8) & 0xf800) |
	      ((g << 3) & 0x07e0) |
	      ((r >> 3)         ));
    }
}
