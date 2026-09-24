static int h2c_send_strm_wu(struct h2c *h2c)
{
	int ret = 1;

	if (h2c->rcvd_s <= 0)
		return 1;

	/* send WU for the stream */
	ret = h2c_send_window_update(h2c, h2c->dsi, h2c->rcvd_s);
	if (ret > 0)
		h2c->rcvd_s = 0;

	return ret;
}
