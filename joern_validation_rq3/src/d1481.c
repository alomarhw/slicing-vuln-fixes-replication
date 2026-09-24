static int sysvipc_msg_proc_show(struct seq_file *s, void *it)
{
	struct user_namespace *user_ns = seq_user_ns(s);
	struct msg_queue *msq = it;

	return seq_printf(s,
			"%10d %10d  %4o  %10lu %10lu %5u %5u %5u %5u %5u %5u %10lu %10lu %10lu\n",
			msq->q_perm.key,
			msq->q_perm.id,
			msq->q_perm.mode,
			msq->q_cbytes,
			msq->q_qnum,
			msq->q_lspid,
			msq->q_lrpid,
			from_kuid_munged(user_ns, msq->q_perm.uid),
			from_kgid_munged(user_ns, msq->q_perm.gid),
			from_kuid_munged(user_ns, msq->q_perm.cuid),
			from_kgid_munged(user_ns, msq->q_perm.cgid),
			msq->q_stime,
			msq->q_rtime,
			msq->q_ctime);
}
