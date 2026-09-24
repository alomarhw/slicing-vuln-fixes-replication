char *WavpackGetFileExtension (WavpackContext *wpc)
{
    if (wpc && wpc->file_extension [0])
        return wpc->file_extension;
    else
        return "wav";
}
