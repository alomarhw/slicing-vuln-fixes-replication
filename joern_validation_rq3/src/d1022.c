static int check_reg_arg(struct reg_state *regs, u32 regno,
			 enum reg_arg_type t)
{
	if (regno >= MAX_BPF_REG) {
		verbose("R%d is invalid\n", regno);
		return -EINVAL;
	}

	if (t == SRC_OP) {
		/* check whether register used as source operand can be read */
		if (regs[regno].type == NOT_INIT) {
			verbose("R%d !read_ok\n", regno);
			return -EACCES;
		}
	} else {
		/* check whether register used as dest operand can be written to */
		if (regno == BPF_REG_FP) {
			verbose("frame pointer is read only\n");
			return -EACCES;
		}
		if (t == DST_OP)
			mark_reg_unknown_value(regs, regno);
	}
	return 0;
}
