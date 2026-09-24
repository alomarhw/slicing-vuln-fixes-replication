static int auth_scan_scoreboard(void) {
  char *key;
  void *v;
  config_rec *c = NULL;
  pr_scoreboard_entry_t *score = NULL;
  unsigned int cur = 0, ccur = 0, hcur = 0;
  char curr_server_addr[80] = {'\0'};
  const char *client_addr = pr_netaddr_get_ipstr(session.c->remote_addr);

  snprintf(curr_server_addr, sizeof(curr_server_addr), "%s:%d",
    pr_netaddr_get_ipstr(session.c->local_addr), main_server->ServerPort);
  curr_server_addr[sizeof(curr_server_addr)-1] = '\0';

  /* Determine how many users are currently connected */
  if (pr_rewind_scoreboard() < 0) {
    pr_log_pri(PR_LOG_NOTICE, "error rewinding scoreboard: %s",
      strerror(errno));
  }

  while ((score = pr_scoreboard_entry_read()) != NULL) {
    pr_signals_handle();

    /* Make sure it matches our current server */
    if (strcmp(score->sce_server_addr, curr_server_addr) == 0) {
      cur++;

      if (strcmp(score->sce_client_addr, client_addr) == 0)
        hcur++;

      /* Only count up authenticated clients, as per the documentation. */
      if (strncmp(score->sce_user, "(none)", 7) == 0)
        continue;

      /* Note: the class member of the scoreboard entry will never be
       * NULL.  At most, it may be the empty string.
       */
      if (session.conn_class != NULL &&
          strcasecmp(score->sce_class, session.conn_class->cls_name) == 0) {
        ccur++;
      }
    }
  }
  pr_restore_scoreboard();

  key = "client-count";
  (void) pr_table_remove(session.notes, key, NULL);
  v = palloc(session.pool, sizeof(unsigned int));
  *((unsigned int *) v) = cur;

  if (pr_table_add(session.notes, key, v, sizeof(unsigned int)) < 0) {
    if (errno != EEXIST) {
      pr_log_pri(PR_LOG_WARNING,
        "warning: error stashing '%s': %s", key, strerror(errno));
    }
  }

  if (session.conn_class != NULL) {
    key = "class-client-count";
    (void) pr_table_remove(session.notes, key, NULL);
    v = palloc(session.pool, sizeof(unsigned int));
    *((unsigned int *) v) = ccur;

    if (pr_table_add(session.notes, key, v, sizeof(unsigned int)) < 0) {
      if (errno != EEXIST) {
        pr_log_pri(PR_LOG_WARNING,
          "warning: error stashing '%s': %s", key, strerror(errno));
      }
    }
  }

  /* Lookup any configured MaxConnectionsPerHost. */
  c = find_config(main_server->conf, CONF_PARAM, "MaxConnectionsPerHost",
    FALSE);

  if (c) {
    unsigned int *max = c->argv[0];

    if (*max &&
        hcur > *max) {

      char maxstr[20];
      char *msg = "Sorry, the maximum number of connections (%m) for your host "
        "are already connected.";

      pr_event_generate("mod_auth.max-connections-per-host", session.c);

      if (c->argc == 2)
        msg = c->argv[1];

      memset(maxstr, '\0', sizeof(maxstr));
      snprintf(maxstr, sizeof(maxstr), "%u", *max);
      maxstr[sizeof(maxstr)-1] = '\0';

      pr_response_send(R_530, "%s", sreplace(session.pool, msg,
        "%m", maxstr, NULL));

      pr_log_auth(PR_LOG_NOTICE,
        "Connection refused (MaxConnectionsPerHost %u)", *max);
      pr_session_disconnect(&auth_module, PR_SESS_DISCONNECT_CONFIG_ACL,
        "Denied by MaxConnectionsPerHost");
    }
  }

  return 0;
}
