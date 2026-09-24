get_uri_type(const char *uri)
{
	int i;
	const char *hostend, *portbegin;
	char *portend;
	unsigned long port;

	/* According to the HTTP standard
	 * http://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html#sec5.1.2
	 * URI can be an asterisk (*) or should start with slash (relative uri),
	 * or it should start with the protocol (absolute uri). */
	if ((uri[0] == '*') && (uri[1] == '\0')) {
		/* asterisk */
		return 1;
	}

	/* Valid URIs according to RFC 3986
	 * (https://www.ietf.org/rfc/rfc3986.txt)
	 * must only contain reserved characters :/?#[]@!$&'()*+,;=
	 * and unreserved characters A-Z a-z 0-9 and -._~
	 * and % encoded symbols.
	 */
	for (i = 0; uri[i] != 0; i++) {
		if (uri[i] < 33) {
			/* control characters and spaces are invalid */
			return 0;
		}
		if (uri[i] > 126) {
			/* non-ascii characters must be % encoded */
			return 0;
		} else {
			switch (uri[i]) {
			case '"':  /* 34 */
			case '<':  /* 60 */
			case '>':  /* 62 */
			case '\\': /* 92 */
			case '^':  /* 94 */
			case '`':  /* 96 */
			case '{':  /* 123 */
			case '|':  /* 124 */
			case '}':  /* 125 */
				return 0;
			default:
				/* character is ok */
				break;
			}
		}
	}

	/* A relative uri starts with a / character */
	if (uri[0] == '/') {
		/* relative uri */
		return 2;
	}

	/* It could be an absolute uri: */
	/* This function only checks if the uri is valid, not if it is
	 * addressing the current server. So civetweb can also be used
	 * as a proxy server. */
	for (i = 0; abs_uri_protocols[i].proto != NULL; i++) {
		if (mg_strncasecmp(uri,
		                   abs_uri_protocols[i].proto,
		                   abs_uri_protocols[i].proto_len) == 0) {

			hostend = strchr(uri + abs_uri_protocols[i].proto_len, '/');
			if (!hostend) {
				return 0;
			}
			portbegin = strchr(uri + abs_uri_protocols[i].proto_len, ':');
			if (!portbegin) {
				return 3;
			}

			port = strtoul(portbegin + 1, &portend, 10);
			if ((portend != hostend) || (port <= 0) || !is_valid_port(port)) {
				return 0;
			}

			return 4;
		}
	}

	return 0;
}
