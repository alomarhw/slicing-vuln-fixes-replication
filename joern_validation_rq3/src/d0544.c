XmpFileType xmp_files_check_file_format(const char *filePath)
{
    CHECK_PTR(filePath, XMP_FT_UNKNOWN);
    RESET_ERROR;

    XmpFileType file_type = XMP_FT_UNKNOWN;
    try {
        file_type = (XmpFileType)SXMPFiles::CheckFileFormat(filePath);
    }
    catch (const XMP_Error &e) {
        set_error(e);
        return XMP_FT_UNKNOWN;
    }
    return file_type;
}
