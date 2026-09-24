CWD_API int php_sys_readlink(const char *link, char *target, size_t target_len){ /* {{{ */
	HINSTANCE kernel32;
	HANDLE hFile;
	DWORD dwRet;

	typedef BOOL (WINAPI *gfpnh_func)(HANDLE, LPTSTR, DWORD, DWORD);
	gfpnh_func pGetFinalPathNameByHandle;

	kernel32 = LoadLibrary("kernel32.dll");

	if (kernel32) {
		pGetFinalPathNameByHandle = (gfpnh_func)GetProcAddress(kernel32, "GetFinalPathNameByHandleA");
		if (pGetFinalPathNameByHandle == NULL) {
			return -1;
		}
	} else {
		return -1;
	}

	hFile = CreateFile(link,            // file to open
				 GENERIC_READ,          // open for reading
				 FILE_SHARE_READ,       // share for reading
				 NULL,                  // default security
				 OPEN_EXISTING,         // existing file only
				 FILE_FLAG_BACKUP_SEMANTICS, // normal file
				 NULL);                 // no attr. template

	if( hFile == INVALID_HANDLE_VALUE) {
			return -1;
	}

	dwRet = pGetFinalPathNameByHandle(hFile, target, MAXPATHLEN, VOLUME_NAME_DOS);
	if(dwRet >= MAXPATHLEN || dwRet == 0) {
		return -1;
	}

	CloseHandle(hFile);

	if(dwRet > 4) {
		/* Skip first 4 characters if they are "\??\" */
		if(target[0] == '\\' && target[1] == '\\' && target[2] == '?' && target[3] ==  '\\') {
			char tmp[MAXPATHLEN];
			unsigned int offset = 4;
			dwRet -= 4;

			/* \??\UNC\ */
			if (dwRet > 7 && target[4] == 'U' && target[5] == 'N' && target[6] == 'C') {
				offset += 2;
				dwRet -= 2;
				target[offset] = '\\';
			}

			memcpy(tmp, target + offset, dwRet);
			memcpy(target, tmp, dwRet);
		}
	}

	target[dwRet] = '\0';
	return dwRet;
}
/* }}} */
