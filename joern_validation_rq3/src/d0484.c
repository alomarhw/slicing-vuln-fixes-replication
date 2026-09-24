static int init_screen(ScreenPtr pScreen, int i, Bool gpu)
{
    int scanlinepad, format, depth, bitsPerPixel, j, k;

    dixInitScreenSpecificPrivates(pScreen);

    if (!dixAllocatePrivates(&pScreen->devPrivates, PRIVATE_SCREEN)) {
        return -1;
    }
    pScreen->myNum = i;
    if (gpu) {
        pScreen->myNum += GPU_SCREEN_OFFSET;
        pScreen->isGPU = TRUE;
    }
    pScreen->totalPixmapSize = 0;       /* computed in CreateScratchPixmapForScreen */
    pScreen->ClipNotify = 0;    /* for R4 ddx compatibility */
    pScreen->CreateScreenResources = 0;

    xorg_list_init(&pScreen->pixmap_dirty_list);
    xorg_list_init(&pScreen->slave_list);

    /*
     * This loop gets run once for every Screen that gets added,
     * but thats ok.  If the ddx layer initializes the formats
     * one at a time calling AddScreen() after each, then each
     * iteration will make it a little more accurate.  Worst case
     * we do this loop N * numPixmapFormats where N is # of screens.
     * Anyway, this must be called after InitOutput and before the
     * screen init routine is called.
     */
    for (format = 0; format < screenInfo.numPixmapFormats; format++) {
        depth = screenInfo.formats[format].depth;
        bitsPerPixel = screenInfo.formats[format].bitsPerPixel;
        scanlinepad = screenInfo.formats[format].scanlinePad;
        j = indexForBitsPerPixel[bitsPerPixel];
        k = indexForScanlinePad[scanlinepad];
        PixmapWidthPaddingInfo[depth].padPixelsLog2 = answer[j][k];
        PixmapWidthPaddingInfo[depth].padRoundUp =
            (scanlinepad / bitsPerPixel) - 1;
        j = indexForBitsPerPixel[8];    /* bits per byte */
        PixmapWidthPaddingInfo[depth].padBytesLog2 = answer[j][k];
        PixmapWidthPaddingInfo[depth].bitsPerPixel = bitsPerPixel;
        if (answerBytesPerPixel[bitsPerPixel]) {
            PixmapWidthPaddingInfo[depth].notPower2 = 1;
            PixmapWidthPaddingInfo[depth].bytesPerPixel =
                answerBytesPerPixel[bitsPerPixel];
        }
        else {
            PixmapWidthPaddingInfo[depth].notPower2 = 0;
        }
    }
    return 0;
}
