void sspi_SecureHandleFree(SecHandle* handle)
{
	if (!handle)
		return;

	free(handle);
}
