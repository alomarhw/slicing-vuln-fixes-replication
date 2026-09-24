config_phone(
	config_tree *ptree
	)
{
	int		i;
	string_node *	sn;

	i = 0;
	sn = HEAD_PFIFO(ptree->phone);
	for (; sn != NULL; sn = sn->link) {
		/* need to leave array entry for NULL terminator */
		if (i < COUNTOF(sys_phone) - 1) {
			sys_phone[i++] = estrdup(sn->s);
			sys_phone[i] = NULL;
		} else {
			msyslog(LOG_INFO,
				"phone: Number of phone entries exceeds %lu. Ignoring phone %s...",
				(u_long)(COUNTOF(sys_phone) - 1), sn->s);
		}
	}
}
