rend_service_check_private_dir(const or_options_t *options,
                               const rend_service_t *s,
                               int create)
{
  /* Passing a NULL service is a bug */
  if (BUG(!s)) {
    return -1;
  }

  /* Check/create directory */
  if (rend_service_check_private_dir_impl(options, s, create) < 0) {
    return -1;
  }

  /* Check if the hidden service key exists, and was created in a different
   * single onion service mode, and refuse to launch if it has.
   * This is safe to call even when create is false, as it ignores missing
   * keys and directories: they are always valid.
   */
  if (rend_service_verify_single_onion_poison(s, options) < 0) {
    /* We can't use s->service_id here, as the key may not have been loaded */
    log_warn(LD_GENERAL, "We are configured with "
             "HiddenServiceNonAnonymousMode %d, but the hidden "
             "service key in directory %s was created in %s mode. "
             "This is not allowed.",
             rend_service_non_anonymous_mode_enabled(options) ? 1 : 0,
             rend_service_escaped_dir(s),
             rend_service_non_anonymous_mode_enabled(options) ?
             "an anonymous" : "a non-anonymous"
             );
    return -1;
  }

  /* Poison new single onion directories immediately after they are created,
   * so that we never accidentally launch non-anonymous hidden services
   * thinking they are anonymous. Any keys created later will end up with the
   * correct poisoning state.
   */
  if (create && rend_service_non_anonymous_mode_enabled(options)) {
    static int logged_warning = 0;

    if (rend_service_poison_new_single_onion_dir(s, options) < 0) {
      log_warn(LD_GENERAL,"Failed to mark new hidden services as non-anonymous"
               ".");
      return -1;
    }

    if (!logged_warning) {
      /* The keys for these services are linked to the server IP address */
      log_notice(LD_REND, "The configured onion service directories have been "
                 "used in single onion mode. They can not be used for "
                 "anonymous hidden services.");
      logged_warning = 1;
    }
  }

  return 0;
}
