server_child (server * serv)
{
	netstore *ns_server;
	netstore *ns_proxy = NULL;
	netstore *ns_local;
	int port = serv->port;
	int error;
	int sok, psok;
	char *hostname = serv->hostname;
	char *real_hostname = NULL;
	char *ip;
	char *proxy_ip = NULL;
	char *local_ip;
	int connect_port;
	char buf[512];
	char bound = 0;
	int proxy_type = 0;
	char *proxy_host = NULL;
	int proxy_port;

	ns_server = net_store_new ();

	/* is a hostname set? - bind to it */
	if (prefs.hex_net_bind_host[0])
	{
		ns_local = net_store_new ();
		local_ip = net_resolve (ns_local, prefs.hex_net_bind_host, 0, &real_hostname);
		if (local_ip != NULL)
		{
			snprintf (buf, sizeof (buf), "5\n%s\n", local_ip);
			write (serv->childwrite, buf, strlen (buf));
			net_bind (ns_local, serv->sok4, serv->sok6);
			bound = 1;
		} else
		{
			write (serv->childwrite, "7\n", 2);
		}
		net_store_destroy (ns_local);
	}

	if (!serv->dont_use_proxy) /* blocked in serverlist? */
	{
		if (FALSE)
			;
#ifdef USE_LIBPROXY
		else if (prefs.hex_net_proxy_type == 5)
		{
			char **proxy_list;
			char *url, *proxy;

			url = g_strdup_printf ("irc://%s:%d", hostname, port);
			proxy_list = px_proxy_factory_get_proxies (libproxy_factory, url);

			if (proxy_list) {
				/* can use only one */
				proxy = proxy_list[0];
				if (!strncmp (proxy, "direct", 6))
					proxy_type = 0;
				else if (!strncmp (proxy, "http", 4))
					proxy_type = 4;
				else if (!strncmp (proxy, "socks5", 6))
					proxy_type = 3;
				else if (!strncmp (proxy, "socks", 5))
					proxy_type = 2;
			}

			if (proxy_type) {
				char *c;
				c = strchr (proxy, ':') + 3;
				proxy_host = strdup (c);
				c = strchr (proxy_host, ':');
				*c = '\0';
				proxy_port = atoi (c + 1);
			}

			g_strfreev (proxy_list);
			g_free (url);
		}
#endif
		else if (prefs.hex_net_proxy_host[0] &&
			   prefs.hex_net_proxy_type > 0 &&
			   prefs.hex_net_proxy_use != 2) /* proxy is NOT dcc-only */
		{
			proxy_type = prefs.hex_net_proxy_type;
			proxy_host = strdup (prefs.hex_net_proxy_host);
			proxy_port = prefs.hex_net_proxy_port;
		}
	}

	serv->proxy_type = proxy_type;

	/* first resolve where we want to connect to */
	if (proxy_type > 0)
	{
		snprintf (buf, sizeof (buf), "9\n%s\n", proxy_host);
		write (serv->childwrite, buf, strlen (buf));
		ip = net_resolve (ns_server, proxy_host, proxy_port, &real_hostname);
		free (proxy_host);
		if (!ip)
		{
			write (serv->childwrite, "1\n", 2);
			goto xit;
		}
		connect_port = proxy_port;

		/* if using socks4 or MS Proxy, attempt to resolve ip for irc server */
		if ((proxy_type == 2) || (proxy_type == 5))
		{
			ns_proxy = net_store_new ();
			proxy_ip = net_resolve (ns_proxy, hostname, port, &real_hostname);
			if (!proxy_ip)
			{
				write (serv->childwrite, "1\n", 2);
				goto xit;
			}
		} else						  /* otherwise we can just use the hostname */
			proxy_ip = strdup (hostname);
	} else
	{
		ip = net_resolve (ns_server, hostname, port, &real_hostname);
		if (!ip)
		{
			write (serv->childwrite, "1\n", 2);
			goto xit;
		}
		connect_port = port;
	}

	snprintf (buf, sizeof (buf), "3\n%s\n%s\n%d\n",
				 real_hostname, ip, connect_port);
	write (serv->childwrite, buf, strlen (buf));

	if (!serv->dont_use_proxy && (proxy_type == 5))
		error = net_connect (ns_server, serv->proxy_sok4, serv->proxy_sok6, &psok);
	else
	{
		error = net_connect (ns_server, serv->sok4, serv->sok6, &sok);
		psok = sok;
	}

	if (error != 0)
	{
		snprintf (buf, sizeof (buf), "2\n%d\n", sock_error ());
		write (serv->childwrite, buf, strlen (buf));
	} else
	{
		/* connect succeeded */
		if (proxy_ip)
		{
			switch (traverse_proxy (proxy_type, serv->childwrite, psok, proxy_ip, port, &serv->msp_state, ns_proxy, serv->sok4, serv->sok6, &sok, bound))
			{
			case 0:	/* success */
#ifdef USE_MSPROXY
				if (!serv->dont_use_proxy && (proxy_type == 5))
					snprintf (buf, sizeof (buf), "4\n%d %d %d %d %d\n", sok, psok, serv->msp_state.clientid, serv->msp_state.serverid,
						serv->msp_state.seq_sent);
				else
#endif
					snprintf (buf, sizeof (buf), "4\n%d\n", sok);	/* success */
				write (serv->childwrite, buf, strlen (buf));
				break;
			case 1:	/* socks traversal failed */
				write (serv->childwrite, "8\n", 2);
				break;
			}
		} else
		{
			snprintf (buf, sizeof (buf), "4\n%d\n", sok);	/* success */
			write (serv->childwrite, buf, strlen (buf));
		}
	}

xit:

#if defined (USE_IPV6) || defined (WIN32)
	/* this is probably not needed */
	net_store_destroy (ns_server);
	if (ns_proxy)
		net_store_destroy (ns_proxy);
#endif

	/* no need to free ip/real_hostname, this process is exiting */
#ifdef WIN32
	/* under win32 we use a thread -> shared memory, must free! */
	if (proxy_ip)
		free (proxy_ip);
	if (ip)
		free (ip);
	if (real_hostname)
		free (real_hostname);
#endif

	return 0;
	/* cppcheck-suppress memleak */
}
