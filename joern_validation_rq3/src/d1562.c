int kvm_handle_cp15_64(struct kvm_vcpu *vcpu, struct kvm_run *run)
{
	const struct sys_reg_desc *target_specific;
	size_t num;

	target_specific = get_target_table(vcpu->arch.target, false, &num);
	return kvm_handle_cp_64(vcpu,
				cp15_64_regs, ARRAY_SIZE(cp15_64_regs),
				target_specific, num);
}
