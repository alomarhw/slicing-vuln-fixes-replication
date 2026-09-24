_gnutls_server_select_comp_method (gnutls_session_t session,
				   opaque * data, int datalen)
{
  int x, i, j;
  uint8_t *comps;

  x = _gnutls_supported_compression_methods (session, &comps);
  if (x < 0)
    {
      gnutls_assert ();
      return x;
    }

  memset (&session->internals.compression_method, 0,
	  sizeof (gnutls_compression_method_t));

  for (j = 0; j < datalen; j++)
    {
      for (i = 0; i < x; i++)
	{
	  if (comps[i] == data[j])
	    {
	      gnutls_compression_method_t method =
		_gnutls_compression_get_id (comps[i]);

	      session->internals.compression_method = method;
	      gnutls_free (comps);

	      _gnutls_handshake_log
		("HSK[%x]: Selected Compression Method: %s\n", session,
		 gnutls_compression_get_name (session->internals.
					      compression_method));


	      return 0;
	    }
	}
    }

  /* we were not able to find a compatible compression
   * algorithm
   */
  gnutls_free (comps);
  gnutls_assert ();
  return GNUTLS_E_UNKNOWN_COMPRESSION_ALGORITHM;

}
