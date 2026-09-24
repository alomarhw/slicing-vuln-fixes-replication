fbStore_x8b8g8r8 (FbBits *bits, const CARD32 *values, int x, int width, miIndexedPtr indexed)
{
    int i;
    CARD32 *pixel = (CARD32 *)bits + x;
    for (i = 0; i < width; ++i)
        WRITE(pixel++, (READ(values + i) & 0x0000ff00) | ((READ(values + i) >> 16) & 0xff) | ((READ(values + i) & 0xff) << 16));
}
