static void GIFAnimEncode(gdIOCtxPtr fp, int IWidth, int IHeight, int LeftOfs, int TopOfs, int GInterlace, int Transparent, int Delay, int Disposal, int BitsPerPixel, int *Red, int *Green, int *Blue, gdImagePtr im)
{
	int B;
	int ColorMapSize;
	int InitCodeSize;
	int i;
	GifCtx ctx;

	memset(&ctx, 0, sizeof(ctx));

	ctx.Interlace = GInterlace;
	ctx.in_count = 1;

	ColorMapSize = 1 << BitsPerPixel;

	if(LeftOfs < 0) {
		LeftOfs = 0;
	}
	if(TopOfs < 0) {
		TopOfs = 0;
	}
	if(Delay < 0) {
		Delay = 100;
	}
	if(Disposal < 0) {
		Disposal = 1;
	}

	ctx.Width = IWidth;
	ctx.Height = IHeight;

	/* Calculate number of bits we are expecting */
	ctx.CountDown = (long)ctx.Width * (long)ctx.Height;

	/* Indicate which pass we are on (if interlace) */
	ctx.Pass = 0;

	/* The initial code size */
	if(BitsPerPixel <= 1) {
		InitCodeSize = 2;
	} else {
		InitCodeSize = BitsPerPixel;
	}

	/* Set up the current x and y position */
	ctx.curx = ctx.cury = 0;

	/* Write out extension for image animation and looping */
	gdPutC('!', fp);
	gdPutC(0xf9, fp);
	gdPutC(4, fp);
	gdPutC((Transparent >= 0 ? 1 : 0) | (Disposal << 2), fp);
	gdPutC((unsigned char)(Delay & 255), fp);
	gdPutC((unsigned char)((Delay >> 8) & 255), fp);
	gdPutC((unsigned char) Transparent, fp);
	gdPutC(0, fp);

	/* Write an Image separator */
	gdPutC(',', fp);

	/* Write out the Image header */
	gifPutWord(LeftOfs, fp);
	gifPutWord(TopOfs, fp);
	gifPutWord(ctx.Width, fp);
	gifPutWord(ctx.Height, fp);

	/* Indicate that there is a local colour map */
	B = (Red && Green && Blue) ? 0x80 : 0;

	/* OR in the interlacing */
	B |= ctx.Interlace ? 0x40 : 0;

	/* OR in the Bits per Pixel */
	B |= (Red && Green && Blue) ? (BitsPerPixel - 1) : 0;

	/* Write it out */
	gdPutC(B, fp);

	/* Write out the Local Colour Map */
	if(Red && Green && Blue) {
		for(i = 0; i < ColorMapSize; ++i) {
			gdPutC(Red[i], fp);
			gdPutC(Green[i], fp);
			gdPutC(Blue[i], fp);
		}
	}

	/* Write out the initial code size */
	gdPutC(InitCodeSize, fp);

	/* Go and actually compress the data */
	compress(InitCodeSize + 1, fp, im, &ctx);

	/* Write out a Zero-length packet (to end the series) */
	gdPutC(0, fp);
}
