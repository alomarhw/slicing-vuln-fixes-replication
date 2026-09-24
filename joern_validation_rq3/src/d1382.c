int WavpackStoreMD5Sum (WavpackContext *wpc, unsigned char data [16])
{
    return add_to_metadata (wpc, data, 16, (wpc->config.qmode & 0xff) ? ID_ALT_MD5_CHECKSUM : ID_MD5_CHECKSUM);
}
