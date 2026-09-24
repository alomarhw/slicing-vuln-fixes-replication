void CL_ConnectionlessPacket( netadr_t from, msg_t *msg ) {
	char	*s;
	char	*c;
	int challenge = 0;

	MSG_BeginReadingOOB( msg );
	MSG_ReadLong( msg );	// skip the -1

	s = MSG_ReadStringLine( msg );

	Cmd_TokenizeString( s );

	c = Cmd_Argv( 0 );

	Com_DPrintf ("CL packet %s: %s\n", NET_AdrToStringwPort(from), c);

	if (!Q_stricmp(c, "challengeResponse"))
	{
		char *strver;
		int ver;
	
		if (clc.state != CA_CONNECTING)
		{
			Com_DPrintf("Unwanted challenge response received. Ignored.\n");
			return;
		}
		
		c = Cmd_Argv( 2 );
		if(*c)
			challenge = atoi(c);

		strver = Cmd_Argv( 3 );
		if(*strver)
		{
			ver = atoi(strver);
			
			if(ver != com_protocol->integer)
			{
#ifdef LEGACY_PROTOCOL
				if(com_legacyprotocol->integer > 0)
				{
					clc.compat = qtrue;

					Com_Printf(S_COLOR_YELLOW "Warning: Server reports protocol version %d, "
						   "we have %d. Trying legacy protocol %d.\n",
						   ver, com_protocol->integer, com_legacyprotocol->integer);
				}
				else
#endif
				{
					Com_Printf(S_COLOR_YELLOW "Warning: Server reports protocol version %d, we have %d. "
						   "Trying anyways.\n", ver, com_protocol->integer);
				}
			}
		}
#ifdef LEGACY_PROTOCOL
		else
			clc.compat = qtrue;
		
		if(clc.compat)
		{
			if(!NET_CompareAdr(from, clc.serverAddress))
			{
			
				if(!*c || challenge != clc.challenge)
				{
					Com_DPrintf("Challenge response received from unexpected source. Ignored.\n");
					return;
				}
			}
		}
		else
#endif
		{
			if(!*c || challenge != clc.challenge)
			{
				Com_Printf("Bad challenge for challengeResponse. Ignored.\n");
				return;
			}
		}

		clc.challenge = atoi(Cmd_Argv(1));
		clc.state = CA_CHALLENGING;
		clc.connectPacketCount = 0;
		clc.connectTime = -99999;

		clc.serverAddress = from;
		Com_DPrintf ("challengeResponse: %d\n", clc.challenge);
		return;
	}

	if ( !Q_stricmp( c, "connectResponse" ) ) {
		if ( clc.state >= CA_CONNECTED ) {
			Com_Printf( "Dup connect received. Ignored.\n" );
			return;
		}
		if ( clc.state != CA_CHALLENGING ) {
			Com_Printf( "connectResponse packet while not connecting. Ignored.\n" );
			return;
		}
		if ( !NET_CompareAdr( from, clc.serverAddress ) ) {
			Com_Printf( "connectResponse from wrong address. Ignored.\n" );
			return;
		}

#ifdef LEGACY_PROTOCOL
		if(!clc.compat)
#endif
		{
			c = Cmd_Argv(1);

			if(*c)
				challenge = atoi(c);
			else
			{
				Com_Printf("Bad connectResponse received. Ignored.\n");
				return;
			}
			
			if(challenge != clc.challenge)
			{
				Com_Printf("ConnectResponse with bad challenge received. Ignored.\n");
				return;
			}
		}

#ifdef LEGACY_PROTOCOL
		Netchan_Setup(NS_CLIENT, &clc.netchan, from, Cvar_VariableValue("net_qport"),
			      clc.challenge, clc.compat);
#else
		Netchan_Setup(NS_CLIENT, &clc.netchan, from, Cvar_VariableValue("net_qport"),
			      clc.challenge, qfalse);
#endif

		clc.state = CA_CONNECTED;
		clc.lastPacketSentTime = -9999;	// send first packet immediately
		return;
	}

	if ( !Q_stricmp( c, "infoResponse" ) ) {
		CL_ServerInfoPacket( from, msg );
		return;
	}

	if ( !Q_stricmp( c, "statusResponse" ) ) {
		CL_ServerStatusResponse( from, msg );
		return;
	}

	if ( !Q_stricmp( c, "echo" ) ) {
		NET_OutOfBandPrint( NS_CLIENT, from, "%s", Cmd_Argv( 1 ) );
		return;
	}

	if ( !Q_stricmp( c, "keyAuthorize" ) ) {
		return;
	}

	if ( !Q_stricmp( c, "motd" ) ) {
		CL_MotdPacket( from );
		return;
	}

	if ( !Q_stricmp( c, "print" ) ) {
		s = MSG_ReadString( msg );
		
		Q_strncpyz( clc.serverMessage, s, sizeof( clc.serverMessage ) );
		Com_Printf( "%s", s );
		return;
	}

	if ( !Q_strncmp( c, "getserversResponse", 18 ) ) {
		CL_ServersResponsePacket( &from, msg, qfalse );
		return;
	}

	if ( !Q_strncmp(c, "getserversExtResponse", 21) ) {
		CL_ServersResponsePacket( &from, msg, qtrue );
		return;
	}

	Com_DPrintf( "Unknown connectionless packet command.\n" );
}
