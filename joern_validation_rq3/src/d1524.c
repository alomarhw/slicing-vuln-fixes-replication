int ptrace_setregs(struct task_struct *child, void __user *uregs)
{
	struct pt_regs *regs = task_pt_regs(child);
	xtensa_gregset_t *gregset = uregs;
	const unsigned long ps_mask = PS_CALLINC_MASK | PS_OWB_MASK;
	unsigned long ps;
	unsigned long wb;

	if (!access_ok(VERIFY_WRITE, uregs, sizeof(xtensa_gregset_t)))
		return -EIO;

	__get_user(regs->pc, &gregset->pc);
	__get_user(ps, &gregset->ps);
	__get_user(regs->lbeg, &gregset->lbeg);
	__get_user(regs->lend, &gregset->lend);
	__get_user(regs->lcount, &gregset->lcount);
	__get_user(regs->windowstart, &gregset->windowstart);
	__get_user(wb, &gregset->windowbase);

	regs->ps = (regs->ps & ~ps_mask) | (ps & ps_mask) | (1 << PS_EXCM_BIT);

	if (wb >= XCHAL_NUM_AREGS / 4)
		return -EFAULT;

	regs->windowbase = wb;

	if (wb != 0 &&  __copy_from_user(regs->areg + XCHAL_NUM_AREGS - wb * 4,
					 gregset->a, wb * 16))
		return -EFAULT;

	if (__copy_from_user(regs->areg, gregset->a + wb*4, (WSBITS-wb) * 16))
		return -EFAULT;

	return 0;
}
