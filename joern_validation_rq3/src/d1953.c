_PUBLIC_ char *strlower_talloc(TALLOC_CTX *ctx, const char *src)
{
	struct smb_iconv_handle *iconv_handle = get_iconv_handle();
	return strlower_talloc_handle(iconv_handle, ctx, src);
}
