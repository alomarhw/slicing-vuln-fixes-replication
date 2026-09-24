nsPluginInstance::handlePlayerRequests(GIOChannel* iochan, GIOCondition cond)
{

    if ( cond & G_IO_HUP ) {
        gnash::log_debug("Player control socket hang up");
        return false;
    }

    assert(cond & G_IO_IN);

    gnash::log_debug("Checking player requests on FD #%d",
              g_io_channel_unix_get_fd(iochan));

    const size_t buf_size = 1;
    gchar buffer[buf_size];

    do {
        GError* error = 0;
        gsize bytes_read = 0;

        GIOStatus status = g_io_channel_read_chars(iochan, buffer, buf_size,
                                                   &bytes_read, &error);

        switch (status) {
          case G_IO_STATUS_ERROR:
              gnash::log_error("error reading request line: %s",
                               error ? error->message : "unspecified error");
              g_error_free(error);
              return false;
          case G_IO_STATUS_EOF:
              gnash::log_error("EOF (error: %s)", 
                               error ? error->message : "unspecified error");
              g_error_free(error);
              return false;
          case G_IO_STATUS_AGAIN:
              gnash::log_debug("read again");
              continue;
          case G_IO_STATUS_NORMAL:
              _requestbuf.append(buffer, buffer+bytes_read);
#if 0
              gnash::log_debug("Normal read: %s", std::string(buffer, buffer+bytes_read));
#endif
              break;
          default:
              gnash::log_error("Abnormal status!");
              return false;
        }
    } while (g_io_channel_get_buffer_condition(iochan) & G_IO_IN);

    processPlayerRequest();
    
    return true;
}
