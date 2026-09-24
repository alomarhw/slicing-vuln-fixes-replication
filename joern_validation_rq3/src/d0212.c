event_fixture_setup(struct EventFixture *ef, const void* data)
{
    (void) data;

    int socks[2];

    /* make some sockets, fresh for every test */
    if(socketpair(AF_UNIX, SOCK_STREAM, 0, socks) == -1)
    {
      perror("socketpair() failed");
      g_assert(0);
    }

    ef->uzbl_sock = socks[0];
    ef->test_sock = socks[1];

    /* attach uzbl_sock to uzbl's event dispatcher. */
    GIOChannel *iochan = g_io_channel_unix_new(ef->uzbl_sock);
    g_io_channel_set_encoding(iochan, NULL, NULL);

    if(!uzbl.comm.connect_chan)
        uzbl.comm.connect_chan = g_ptr_array_new();
    if(!uzbl.comm.client_chan)
        uzbl.comm.client_chan = g_ptr_array_new();
    g_ptr_array_add(uzbl.comm.client_chan, (gpointer)iochan);
}
