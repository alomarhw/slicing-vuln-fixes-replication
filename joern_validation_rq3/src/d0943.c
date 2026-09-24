static int check_cond_jmp_op(struct bpf_verifier_env *env,
			     struct bpf_insn *insn, int *insn_idx)
{
	struct bpf_verifier_state *other_branch, *this_branch = &env->cur_state;
	struct bpf_reg_state *regs = this_branch->regs, *dst_reg;
	u8 opcode = BPF_OP(insn->code);
	int err;

	if (opcode > BPF_EXIT) {
		verbose("invalid BPF_JMP opcode %x\n", opcode);
		return -EINVAL;
	}

	if (BPF_SRC(insn->code) == BPF_X) {
		if (insn->imm != 0) {
			verbose("BPF_JMP uses reserved fields\n");
			return -EINVAL;
		}

		/* check src1 operand */
		err = check_reg_arg(regs, insn->src_reg, SRC_OP);
		if (err)
			return err;

		if (is_pointer_value(env, insn->src_reg)) {
			verbose("R%d pointer comparison prohibited\n",
				insn->src_reg);
			return -EACCES;
		}
	} else {
		if (insn->src_reg != BPF_REG_0) {
			verbose("BPF_JMP uses reserved fields\n");
			return -EINVAL;
		}
	}

	/* check src2 operand */
	err = check_reg_arg(regs, insn->dst_reg, SRC_OP);
	if (err)
		return err;

	dst_reg = &regs[insn->dst_reg];

	/* detect if R == 0 where R was initialized to zero earlier */
	if (BPF_SRC(insn->code) == BPF_K &&
	    (opcode == BPF_JEQ || opcode == BPF_JNE) &&
	    dst_reg->type == CONST_IMM && dst_reg->imm == insn->imm) {
		if (opcode == BPF_JEQ) {
			/* if (imm == imm) goto pc+off;
			 * only follow the goto, ignore fall-through
			 */
			*insn_idx += insn->off;
			return 0;
		} else {
			/* if (imm != imm) goto pc+off;
			 * only follow fall-through branch, since
			 * that's where the program will go
			 */
			return 0;
		}
	}

	other_branch = push_stack(env, *insn_idx + insn->off + 1, *insn_idx);
	if (!other_branch)
		return -EFAULT;

	/* detect if we are comparing against a constant value so we can adjust
	 * our min/max values for our dst register.
	 */
	if (BPF_SRC(insn->code) == BPF_X) {
		if (regs[insn->src_reg].type == CONST_IMM)
			reg_set_min_max(&other_branch->regs[insn->dst_reg],
					dst_reg, regs[insn->src_reg].imm,
					opcode);
		else if (dst_reg->type == CONST_IMM)
			reg_set_min_max_inv(&other_branch->regs[insn->src_reg],
					    &regs[insn->src_reg], dst_reg->imm,
					    opcode);
	} else {
		reg_set_min_max(&other_branch->regs[insn->dst_reg],
					dst_reg, insn->imm, opcode);
	}

	/* detect if R == 0 where R is returned from bpf_map_lookup_elem() */
	if (BPF_SRC(insn->code) == BPF_K &&
	    insn->imm == 0 && (opcode == BPF_JEQ || opcode == BPF_JNE) &&
	    dst_reg->type == PTR_TO_MAP_VALUE_OR_NULL) {
		/* Mark all identical map registers in each branch as either
		 * safe or unknown depending R == 0 or R != 0 conditional.
		 */
		mark_map_regs(this_branch, insn->dst_reg,
			      opcode == BPF_JEQ ? PTR_TO_MAP_VALUE : UNKNOWN_VALUE);
		mark_map_regs(other_branch, insn->dst_reg,
			      opcode == BPF_JEQ ? UNKNOWN_VALUE : PTR_TO_MAP_VALUE);
	} else if (BPF_SRC(insn->code) == BPF_X && opcode == BPF_JGT &&
		   dst_reg->type == PTR_TO_PACKET &&
		   regs[insn->src_reg].type == PTR_TO_PACKET_END) {
		find_good_pkt_pointers(this_branch, dst_reg);
	} else if (BPF_SRC(insn->code) == BPF_X && opcode == BPF_JGE &&
		   dst_reg->type == PTR_TO_PACKET_END &&
		   regs[insn->src_reg].type == PTR_TO_PACKET) {
		find_good_pkt_pointers(other_branch, &regs[insn->src_reg]);
	} else if (is_pointer_value(env, insn->dst_reg)) {
		verbose("R%d pointer comparison prohibited\n", insn->dst_reg);
		return -EACCES;
	}
	if (log_level)
		print_verifier_state(this_branch);
	return 0;
}
