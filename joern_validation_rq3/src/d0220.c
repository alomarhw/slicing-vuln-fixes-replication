static apr_byte_t oidc_get_remote_user(request_rec *r, oidc_cfg *c,
		oidc_provider_t *provider, oidc_jwt_t *jwt, char **user,
		const char *s_claims) {

	char *issuer = provider->issuer;
	char *claim_name = apr_pstrdup(r->pool, c->remote_user_claim.claim_name);
	int n = strlen(claim_name);
	int post_fix_with_issuer = (claim_name[n - 1] == '@');
	if (post_fix_with_issuer) {
		claim_name[n - 1] = '\0';
		issuer =
				(strstr(issuer, "https://") == NULL) ?
						apr_pstrdup(r->pool, issuer) :
						apr_pstrdup(r->pool, issuer + strlen("https://"));
	}

	/* extract the username claim (default: "sub") from the id_token payload or user claims */
	char *username = NULL;
	json_error_t json_error;
	json_t *claims = json_loads(s_claims, 0, &json_error);
	if (claims == NULL) {
		username = apr_pstrdup(r->pool,
				json_string_value(
						json_object_get(jwt->payload.value.json, claim_name)));
	} else {
		oidc_util_json_merge(jwt->payload.value.json, claims);
		username = apr_pstrdup(r->pool,
				json_string_value(json_object_get(claims, claim_name)));
		json_decref(claims);
	}

	if (username == NULL) {
		oidc_error(r,
				"OIDCRemoteUserClaim is set to \"%s\", but the id_token JSON payload and user claims did not contain a \"%s\" string",
				c->remote_user_claim.claim_name, claim_name);
		*user = NULL;
		return FALSE;
	}

	/* set the unique username in the session (will propagate to r->user/REMOTE_USER) */
	*user = post_fix_with_issuer ?
			apr_psprintf(r->pool, "%s@%s", username, issuer) : username;

	if (c->remote_user_claim.reg_exp != NULL) {

		char *error_str = NULL;
		if (oidc_util_regexp_first_match(r->pool, *user,
				c->remote_user_claim.reg_exp, user, &error_str) == FALSE) {
			oidc_error(r, "oidc_util_regexp_first_match failed: %s", error_str);
			*user = NULL;
			return FALSE;
		}
	}

	oidc_debug(r, "set user to \"%s\"", *user);

	return TRUE;
}
