_dbus_lookup_session_address (dbus_bool_t *supported,
                              DBusString  *address,
                              DBusError   *error)
{
#ifdef DBUS_ENABLE_LAUNCHD
  *supported = TRUE;
  return _dbus_lookup_session_address_launchd (address, error);
#else
  /* On non-Mac Unix platforms, if the session address isn't already
   * set in DBUS_SESSION_BUS_ADDRESS environment variable, we punt and
   * fall back to the autolaunch: global default; see
   * init_session_address in dbus/dbus-bus.c. */
  *supported = FALSE;
  return TRUE;
#endif
}
