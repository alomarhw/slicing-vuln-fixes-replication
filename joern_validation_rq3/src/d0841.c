static void svm_set_cr3(struct kvm_vcpu *vcpu, unsigned long root)
{
	struct vcpu_svm *svm = to_svm(vcpu);

	svm->vmcb->save.cr3 = root;
	mark_dirty(svm->vmcb, VMCB_CR);
	svm_flush_tlb(vcpu);
}
