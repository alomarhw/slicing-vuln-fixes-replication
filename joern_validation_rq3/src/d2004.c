static int hfs_decompress_noncompressed_block(char* rawBuf, uint32_t len, char* uncBuf, uint64_t* uncLen) {
    if (tsk_verbose)
        tsk_fprintf(stderr,
           "%s: Copying an uncompressed compression unit\n", __func__);

    if ((len - 1) > COMPRESSION_UNIT_SIZE) {
        error_detected(TSK_ERR_FS_READ,
            "%s: uncompressed block length %u is longer "
            "than compression unit size %u", __func__, len - 1,
            COMPRESSION_UNIT_SIZE);
        return 0;
    }
    memcpy(uncBuf, rawBuf + 1, len - 1);
    *uncLen = len - 1;
    return 1;
}
