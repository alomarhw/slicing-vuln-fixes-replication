static void svm_cpuid_update(struct kvm_vcpu *vcpu)
{
	struct vcpu_svm *svm = to_svm(vcpu);

	/* Update nrips enabled cache */
	svm->nrips_enabled = !!guest_cpuid_has_nrips(&svm->vcpu);
}
