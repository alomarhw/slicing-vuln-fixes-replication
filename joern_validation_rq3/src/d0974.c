static bool is_mmio_page_fault(struct kvm_vcpu *vcpu, gva_t addr)
{
	if (vcpu->arch.mmu.direct_map || mmu_is_nested(vcpu))
		return vcpu_match_mmio_gpa(vcpu, addr);

	return vcpu_match_mmio_gva(vcpu, addr);
}
