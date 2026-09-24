bool kvm_make_all_cpus_request(struct kvm *kvm, unsigned int req)
{
	cpumask_var_t cpus;
	bool called;

	zalloc_cpumask_var(&cpus, GFP_ATOMIC);

	called = kvm_make_vcpus_request_mask(kvm, req, NULL, cpus);

	free_cpumask_var(cpus);
	return called;
}
