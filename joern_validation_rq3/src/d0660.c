char * gf_get_default_cache_directory() {
	char szPath[GF_MAX_PATH];
	char* root_tmp;
	size_t len;
#ifdef _WIN32_WCE
	strcpy(szPath, "\\windows\\temp" );
#elif defined(WIN32)
	GetTempPath(GF_MAX_PATH, szPath);
#else
	strcpy(szPath, "/tmp");
#endif

	root_tmp = gf_strdup(szPath);

	len = strlen(szPath);
	if (szPath[len-1] != GF_PATH_SEPARATOR) {
		szPath[len] = GF_PATH_SEPARATOR;
		szPath[len+1] = 0;
	}

	strcat(szPath, "gpac_cache");

	if ( !gf_dir_exists(szPath) && gf_mkdir(szPath)!=GF_OK ) {
		return root_tmp;
	}

	gf_free(root_tmp);
	return gf_strdup(szPath);
}
