static bool login_via_cert(PgSocket *client)
{
	struct tls *tls = client->sbuf.tls;
	struct tls_cert *cert;
	struct tls_cert_dname *subj;

	if (!tls) {
		disconnect_client(client, true, "TLS connection required");
		return false;
	}
	if (tls_get_peer_cert(client->sbuf.tls, &cert, NULL) < 0 || !cert) {
		disconnect_client(client, true, "TLS client certificate required");
		return false;
	}

	subj = &cert->subject;
	log_debug("TLS cert login: CN=%s/C=%s/L=%s/ST=%s/O=%s/OU=%s",
		  subj->common_name ? subj->common_name : "(null)",
		  subj->country_name ? subj->country_name : "(null)",
		  subj->locality_name ? subj->locality_name : "(null)",
		  subj->state_or_province_name ? subj->state_or_province_name : "(null)",
		  subj->organization_name ? subj->organization_name : "(null)",
		  subj->organizational_unit_name ? subj->organizational_unit_name : "(null)");
	if (!subj->common_name) {
		disconnect_client(client, true, "Invalid TLS certificate");
		goto fail;
	}
	if (strcmp(subj->common_name, client->auth_user->name) != 0) {
		disconnect_client(client, true, "TLS certificate name mismatch");
		goto fail;
	}
	tls_cert_free(cert);

	/* login successful */
	return finish_client_login(client);
fail:
	tls_cert_free(cert);
	return false;
}
