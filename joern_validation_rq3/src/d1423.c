aiff_close (SF_PRIVATE *psf)
{	AIFF_PRIVATE *paiff = psf->container_data ;

	if (paiff != NULL && paiff->markstr != NULL)
	{	free (paiff->markstr) ;
		paiff->markstr = NULL ;
		} ;

	if (psf->file.mode == SFM_WRITE || psf->file.mode == SFM_RDWR)
	{	aiff_write_tailer (psf) ;
		aiff_write_header (psf, SF_TRUE) ;
		} ;

	return 0 ;
} /* aiff_close */
