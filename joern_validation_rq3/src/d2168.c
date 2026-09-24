static int sched_autogroup_show(struct seq_file *m, void *v)
{
	struct inode *inode = m->private;
	struct task_struct *p;

	p = get_proc_task(inode);
	if (!p)
		return -ESRCH;
	proc_sched_autogroup_show_task(p, m);

	put_task_struct(p);

	return 0;
}
