sec_parse_crypt_info(STREAM s, uint32 * rc4_key_size,
		     uint8 ** server_random, uint8 * modulus, uint8 * exponent)
{
	uint32 crypt_level, random_len, rsa_info_len;
	uint32 cacert_len, cert_len, flags;
	RDSSL_CERT *cacert, *server_cert;
	RDSSL_RKEY *server_public_key;
	uint16 tag, length;
	uint8 *next_tag, *end;

	logger(Protocol, Debug, "%s()", __func__);

	in_uint32_le(s, *rc4_key_size);	/* 1 = 40-bit, 2 = 128-bit */
	in_uint32_le(s, crypt_level);	/* 1 = low, 2 = medium, 3 = high */
	if (crypt_level == 0)
	{
		/* no encryption */
		logger(Protocol, Debug, "sec_parse_crypt_info(), got ENCRYPTION_LEVEL_NONE");
		return False;
	}

	in_uint32_le(s, random_len);
	in_uint32_le(s, rsa_info_len);

	if (random_len != SEC_RANDOM_SIZE)
	{
		logger(Protocol, Error, "sec_parse_crypt_info(), got random len %d, expected %d",
		       random_len, SEC_RANDOM_SIZE);
		return False;
	}

	in_uint8p(s, *server_random, random_len);

	/* RSA info */
	end = s->p + rsa_info_len;
	if (end > s->end)
	{
		logger(Protocol, Error, "sec_parse_crypt_info(), end > s->end");
		return False;
	}

	in_uint32_le(s, flags);	/* 1 = RDP4-style, 0x80000002 = X.509 */
	if (flags & 1)
	{
		logger(Protocol, Debug,
		       "sec_parse_crypt_info(), We're going for the RDP4-style encryption");
		in_uint8s(s, 8);	/* unknown */

		while (s->p < end)
		{
			in_uint16_le(s, tag);
			in_uint16_le(s, length);

			next_tag = s->p + length;

			switch (tag)
			{
				case SEC_TAG_PUBKEY:
					if (!sec_parse_public_key(s, modulus, exponent))
					{
						logger(Protocol, Error,
						       "sec_parse_crypt_info(), invalid public key");
						return False;
					}
					logger(Protocol, Debug,
					       "sec_parse_crypt_info(), got public key");

					break;

				case SEC_TAG_KEYSIG:
					if (!sec_parse_public_sig(s, length, modulus, exponent))
					{
						logger(Protocol, Error,
						       "sec_parse_crypt_info(), invalid public sig");
						return False;
					}
					break;

				default:
					logger(Protocol, Warning,
					       "sec_parse_crypt_info(), unhandled crypt tag 0x%x",
					       tag);
			}

			s->p = next_tag;
		}
	}
	else
	{
		uint32 certcount;

		logger(Protocol, Debug,
		       "sec_parse_crypt_info(), We're going for the RDP5-style encryption");
		in_uint32_le(s, certcount);	/* Number of certificates */
		if (certcount < 2)
		{
			logger(Protocol, Error,
			       "sec_parse_crypt_info(), server didn't send enough x509 certificates");
			return False;
		}
		for (; certcount > 2; certcount--)
		{		/* ignore all the certificates between the root and the signing CA */
			uint32 ignorelen;
			RDSSL_CERT *ignorecert;

			in_uint32_le(s, ignorelen);
			ignorecert = rdssl_cert_read(s->p, ignorelen);
			in_uint8s(s, ignorelen);
			if (ignorecert == NULL)
			{	/* XXX: error out? */
				logger(Protocol, Error,
				       "sec_parse_crypt_info(), got a bad cert: this will probably screw up the rest of the communication");
			}
		}
		/* Do da funky X.509 stuffy

		   "How did I find out about this?  I looked up and saw a
		   bright light and when I came to I had a scar on my forehead
		   and knew about X.500"
		   - Peter Gutman in a early version of 
		   http://www.cs.auckland.ac.nz/~pgut001/pubs/x509guide.txt
		 */
		in_uint32_le(s, cacert_len);
		logger(Protocol, Debug,
		       "sec_parse_crypt_info(), server CA Certificate length is %d", cacert_len);
		cacert = rdssl_cert_read(s->p, cacert_len);
		in_uint8s(s, cacert_len);
		if (NULL == cacert)
		{
			logger(Protocol, Error,
			       "sec_parse_crypt_info(), couldn't load CA Certificate from server");
			return False;
		}
		in_uint32_le(s, cert_len);
		logger(Protocol, Debug, "sec_parse_crypt_info(), certificate length is %d",
		       cert_len);
		server_cert = rdssl_cert_read(s->p, cert_len);
		in_uint8s(s, cert_len);
		if (NULL == server_cert)
		{
			rdssl_cert_free(cacert);
			logger(Protocol, Error,
			       "sec_parse_crypt_info(), couldn't load Certificate from server");
			return False;
		}
		if (!rdssl_certs_ok(server_cert, cacert))
		{
			rdssl_cert_free(server_cert);
			rdssl_cert_free(cacert);
			logger(Protocol, Error,
			       "sec_parse_crypt_info(), security error, CA Certificate invalid");
			return False;
		}
		rdssl_cert_free(cacert);
		in_uint8s(s, 16);	/* Padding */
		server_public_key = rdssl_cert_to_rkey(server_cert, &g_server_public_key_len);
		if (NULL == server_public_key)
		{
			logger(Protocol, Debug,
			       "sec_parse_crypt_info(). failed to parse X509 correctly");
			rdssl_cert_free(server_cert);
			return False;
		}
		rdssl_cert_free(server_cert);
		if ((g_server_public_key_len < SEC_MODULUS_SIZE) ||
		    (g_server_public_key_len > SEC_MAX_MODULUS_SIZE))
		{
			logger(Protocol, Error,
			       "sec_parse_crypt_info(), bad server public key size (%u bits)",
			       g_server_public_key_len * 8);
			rdssl_rkey_free(server_public_key);
			return False;
		}
		if (rdssl_rkey_get_exp_mod(server_public_key, exponent, SEC_EXPONENT_SIZE,
					   modulus, SEC_MAX_MODULUS_SIZE) != 0)
		{
			logger(Protocol, Error,
			       "sec_parse_crypt_info(), problem extracting RSA exponent, modulus");
			rdssl_rkey_free(server_public_key);
			return False;
		}
		rdssl_rkey_free(server_public_key);
		return True;	/* There's some garbage here we don't care about */
	}
	return s_check_end(s);
}
