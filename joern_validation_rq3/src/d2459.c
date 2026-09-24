psf_fsync (SF_PRIVATE *psf)
{	FlushFileBuffers (psf->file.handle) ;
} /* psf_fsync */
