static int oidc_check_max_session_duration(request_rec *r, oidc_cfg *cfg,
		oidc_session_t *session) {

	/* get the session expiry from the session data */
	apr_time_t session_expires = oidc_session_get_session_expires(r, session);

	/* check the expire timestamp against the current time */
	if (apr_time_now() > session_expires) {
		oidc_warn(r, "maximum session duration exceeded for user: %s",
				session->remote_user);
		oidc_session_kill(r, session);
		return oidc_handle_unauthenticated_user(r, cfg);
	}

	/* log message about max session duration */
	oidc_log_session_expires(r, "session max lifetime", session_expires);

	return OK;
}
