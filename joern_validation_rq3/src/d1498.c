static struct fpm_child_s *fpm_resources_prepare(struct fpm_worker_pool_s *wp) /* {{{ */
{
	struct fpm_child_s *c;

	c = fpm_child_alloc();

	if (!c) {
		zlog(ZLOG_ERROR, "[pool %s] unable to malloc new child", wp->config->name);
		return 0;
	}

	c->wp = wp;
	c->fd_stdout = -1; c->fd_stderr = -1;

	if (0 > fpm_stdio_prepare_pipes(c)) {
		fpm_child_free(c);
		return 0;
	}

	if (0 > fpm_scoreboard_proc_alloc(wp->scoreboard, &c->scoreboard_i)) {
		fpm_stdio_discard_pipes(c);
		fpm_child_free(c);
		return 0;
	}

	return c;
}
/* }}} */
