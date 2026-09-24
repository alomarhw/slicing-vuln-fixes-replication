int lxc_clear_groups(struct lxc_conf *c)
{
	struct lxc_list *it,*next;

	lxc_list_for_each_safe(it, &c->groups, next) {
		lxc_list_del(it);
		free(it->elem);
		free(it);
	}
	return 0;
}
