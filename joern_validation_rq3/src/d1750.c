void CL_InitDownloads( void ) {
#ifndef PRE_RELEASE_DEMO
	char missingfiles[1024];
	char *dir = FS_ShiftStr( AUTOUPDATE_DIR, AUTOUPDATE_DIR_SHIFT );

	if ( autoupdateStarted && NET_CompareAdr( cls.autoupdateServer, clc.serverAddress ) ) {
		if ( strlen( cl_updatefiles->string ) > 4 ) {
			Q_strncpyz( autoupdateFilename, cl_updatefiles->string, sizeof( autoupdateFilename ) );
			Q_strncpyz( clc.downloadList, va( "@%s/%s@%s/%s", dir, cl_updatefiles->string, dir, cl_updatefiles->string ), MAX_INFO_STRING );
			clc.state = CA_CONNECTED;
			CL_NextDownload();
			return;
		}
	} else {
		if ( !(cl_allowDownload->integer & DLF_ENABLE) ) {

			if ( FS_ComparePaks( missingfiles, sizeof( missingfiles ), qfalse ) ) {
				Cvar_Set( "com_missingFiles", missingfiles );
			} else {
				Cvar_Set( "com_missingFiles", "" );
			}
			Com_Printf( "\nWARNING: You are missing some files referenced by the server:\n%s"
						"You might not be able to join the game\n"
						"Go to the setting menu to turn on autodownload, or get the file elsewhere\n\n", missingfiles );
		}
		else if ( FS_ComparePaks( clc.downloadList, sizeof( clc.downloadList ), qtrue ) ) {
			Com_Printf( CL_TranslateStringBuf( "Need paks: %s\n" ), clc.downloadList );

			if ( *clc.downloadList ) {
				clc.state = CA_CONNECTED;

				*clc.downloadTempName = *clc.downloadName = 0;
				Cvar_Set( "cl_downloadName", "" );

				CL_NextDownload();
				return;
			}
		}
	}
#endif

	CL_DownloadsComplete();
}
