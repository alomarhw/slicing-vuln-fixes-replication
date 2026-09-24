fbFetchPixel_x1r5g5b5 (const FbBits *bits, int offset, miIndexedPtr indexed)
{
    CARD32  pixel = READ((CARD16 *) bits + offset);
    CARD32  r,g,b;

    r = ((pixel & 0x7c00) | ((pixel & 0x7000) >> 5)) << 9;
    g = ((pixel & 0x03e0) | ((pixel & 0x0380) >> 5)) << 6;
    b = ((pixel & 0x001c) | ((pixel & 0x001f) << 5)) >> 2;
    return (0xff000000 | r | g | b);
}
