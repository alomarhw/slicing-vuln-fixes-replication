psf_close_handle (HANDLE handle)
{	if (handle == NULL)
		return 0 ;

	if (CloseHandle (handle) == 0)
		return -1 ;

	return 0 ;
} /* psf_close_handle */
