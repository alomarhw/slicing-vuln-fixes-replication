FcCacheIsMmapSafe (int fd)
{
    enum {
      MMAP_NOT_INITIALIZED = 0,
      MMAP_USE,
      MMAP_DONT_USE,
      MMAP_CHECK_FS,
    } status;
    static void *static_status;

    status = (intptr_t) fc_atomic_ptr_get (&static_status);

    if (status == MMAP_NOT_INITIALIZED)
    {
	const char *env = getenv ("FONTCONFIG_USE_MMAP");
	FcBool use;
	if (env && FcNameBool ((const FcChar8 *) env, &use))
	    status =  use ? MMAP_USE : MMAP_DONT_USE;
	else
	    status = MMAP_CHECK_FS;
	(void) fc_atomic_ptr_cmpexch (&static_status, NULL, (void *) status);
    }

    if (status == MMAP_CHECK_FS)
	return FcIsFsMmapSafe (fd);
    else
	return status == MMAP_USE;

}
