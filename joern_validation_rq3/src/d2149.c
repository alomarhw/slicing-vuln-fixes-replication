static int netsnmp_session_set_security(struct snmp_session *session, char *sec_level, char *auth_protocol, char *auth_passphrase, char *priv_protocol, char *priv_passphrase, char *contextName, char *contextEngineID TSRMLS_DC)
{

	/* Setting the security level. */
	if (netsnmp_session_set_sec_level(session, sec_level)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid security level '%s'", sec_level);
		return (-1);
	}

	if (session->securityLevel == SNMP_SEC_LEVEL_AUTHNOPRIV || session->securityLevel == SNMP_SEC_LEVEL_AUTHPRIV) {

		/* Setting the authentication protocol. */
		if (netsnmp_session_set_auth_protocol(session, auth_protocol TSRMLS_CC)) {
			/* Warning message sent already, just bail out */
			return (-1);
		}

		/* Setting the authentication passphrase. */
		if (netsnmp_session_gen_auth_key(session, auth_passphrase TSRMLS_CC)) {
			/* Warning message sent already, just bail out */
			return (-1);
		}

		if (session->securityLevel == SNMP_SEC_LEVEL_AUTHPRIV) {
			/* Setting the security protocol. */
			if (netsnmp_session_set_sec_protocol(session, priv_protocol TSRMLS_CC)) {
				/* Warning message sent already, just bail out */
				return (-1);
			}

			/* Setting the security protocol passphrase. */
			if (netsnmp_session_gen_sec_key(session, priv_passphrase TSRMLS_CC)) {
				/* Warning message sent already, just bail out */
				return (-1);
			}
		}
	}

	/* Setting contextName if specified */
	if (contextName) {
		session->contextName = contextName;
		session->contextNameLen = strlen(contextName);
	}

	/* Setting contextEngineIS if specified */
	if (contextEngineID && strlen(contextEngineID) && netsnmp_session_set_contextEngineID(session, contextEngineID TSRMLS_CC)) {
		/* Warning message sent already, just bail out */
		return (-1);
	}

	return (0);
}
