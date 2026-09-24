static int tm_ppr_get(struct task_struct *target,
		      const struct user_regset *regset,
		      unsigned int pos, unsigned int count,
		      void *kbuf, void __user *ubuf)
{
	int ret;

	if (!cpu_has_feature(CPU_FTR_TM))
		return -ENODEV;

	if (!MSR_TM_ACTIVE(target->thread.regs->msr))
		return -ENODATA;

	ret = user_regset_copyout(&pos, &count, &kbuf, &ubuf,
				&target->thread.tm_ppr, 0, sizeof(u64));
	return ret;
}
