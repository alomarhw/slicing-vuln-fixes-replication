static int apparmor_ptrace_traceme(struct task_struct *parent)
{
	return aa_ptrace(parent, current, PTRACE_MODE_ATTACH);
}
