const char *get_reason(unsigned int status)
{
	switch (status) {
	case 100: return "Continue";
	case 101: return "Switching Protocols";
	case 102: return "Processing";
	case 200: return "OK";
	case 201: return "Created";
	case 202: return "Accepted";
	case 203: return "Non-Authoritative Information";
	case 204: return "No Content";
	case 205: return "Reset Content";
	case 206: return "Partial Content";
	case 207: return "Multi-Status";
	case 210: return "Content Different";
	case 226: return "IM Used";
	case 300: return "Multiple Choices";
	case 301: return "Moved Permanently";
	case 302: return "Moved Temporarily";
	case 303: return "See Other";
	case 304: return "Not Modified";
	case 305: return "Use Proxy";
	case 307: return "Temporary Redirect";
	case 308: return "Permanent Redirect";
	case 310: return "Too many Redirects";
	case 400: return "Bad Request";
	case 401: return "Unauthorized";
	case 402: return "Payment Required";
	case 403: return "Forbidden";
	case 404: return "Not Found";
	case 405: return "Method Not Allowed";
	case 406: return "Not Acceptable";
	case 407: return "Proxy Authentication Required";
	case 408: return "Request Time-out";
	case 409: return "Conflict";
	case 410: return "Gone";
	case 411: return "Length Required";
	case 412: return "Precondition Failed";
	case 413: return "Request Entity Too Large";
	case 414: return "Request-URI Too Long";
	case 415: return "Unsupported Media Type";
	case 416: return "Requested range unsatisfiable";
	case 417: return "Expectation failed";
	case 418: return "I'm a teapot";
	case 422: return "Unprocessable entity";
	case 423: return "Locked";
	case 424: return "Method failure";
	case 425: return "Too Early";
	case 426: return "Upgrade Required";
	case 428: return "Precondition Required";
	case 429: return "Too Many Requests";
	case 431: return "Request Header Fields Too Large";
	case 449: return "Retry With";
	case 450: return "Blocked by Windows Parental Controls";
	case 451: return "Unavailable For Legal Reasons";
	case 456: return "Unrecoverable Error";
	case 499: return "client has closed connection";
	case 500: return "Internal Server Error";
	case 501: return "Not Implemented";
	case 502: return "Bad Gateway or Proxy Error";
	case 503: return "Service Unavailable";
	case 504: return "Gateway Time-out";
	case 505: return "HTTP Version not supported";
	case 506: return "Variant also negociate";
	case 507: return "Insufficient storage";
	case 508: return "Loop detected";
	case 509: return "Bandwidth Limit Exceeded";
	case 510: return "Not extended";
	case 511: return "Network authentication required";
	case 520: return "Web server is returning an unknown error";
	default:
		switch (status) {
		case 100 ... 199: return "Informational";
		case 200 ... 299: return "Success";
		case 300 ... 399: return "Redirection";
		case 400 ... 499: return "Client Error";
		case 500 ... 599: return "Server Error";
		default:          return "Other";
		}
	}
}
