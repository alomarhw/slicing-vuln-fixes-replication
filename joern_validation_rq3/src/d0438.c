static void TIFFReadDirEntryCheckedByte(TIFF* tif, TIFFDirEntry* direntry, uint8* value)
{
	(void) tif;
	*value=*(uint8*)(&direntry->tdir_offset);
}
