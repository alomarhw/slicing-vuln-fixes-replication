static void *netlink_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	++*pos;
	return __netlink_seq_next(seq);
}
