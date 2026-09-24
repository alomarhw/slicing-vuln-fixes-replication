static char *oidc_get_state_cookie_name(request_rec *r, const char *state) {
	return apr_psprintf(r->pool, "%s%s", OIDC_STATE_COOKIE_PREFIX, state);
}
